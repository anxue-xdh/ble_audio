#!/usr/bin/env python3
"""
Beta Test Automation Script for BLE Audio System
Automates testing procedures to address beta testing challenges
Author: Beta Test Team
Date: 2024-12-20
"""

import serial
import time
import threading
import json
import datetime
import argparse
import re
import sys
import os

class BLEAudioTester:
    def __init__(self, serial_port, baud_rate=115200):
        """Initialize the BLE audio tester"""
        self.serial_port = serial_port
        self.baud_rate = baud_rate
        self.serial_conn = None
        self.test_results = []
        self.running = False
        self.log_file = None
        self.stats = {
            'total_tests': 0,
            'passed_tests': 0,
            'failed_tests': 0,
            'core_obliterated_errors': 0,
            'sd_card_errors': 0,
            'audio_dropouts': 0,
            'test_start_time': None,
            'test_end_time': None
        }
        
    def connect(self):
        """Connect to the serial port"""
        try:
            self.serial_conn = serial.Serial(
                self.serial_port,
                self.baud_rate,
                timeout=1
            )
            print(f"Connected to {self.serial_port} at {self.baud_rate} baud")
            return True
        except Exception as e:
            print(f"Failed to connect to {self.serial_port}: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from the serial port"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            print("Disconnected from serial port")
    
    def send_command(self, command):
        """Send a command to the device"""
        if not self.serial_conn or not self.serial_conn.is_open:
            print("Serial connection not available")
            return False
        
        try:
            self.serial_conn.write(f"{command}\r\n".encode())
            self.log_message(f"SENT: {command}")
            return True
        except Exception as e:
            print(f"Failed to send command: {e}")
            return False
    
    def read_response(self, timeout=5):
        """Read response from device"""
        if not self.serial_conn or not self.serial_conn.is_open:
            return None
        
        start_time = time.time()
        response_lines = []
        
        while time.time() - start_time < timeout:
            try:
                if self.serial_conn.in_waiting:
                    line = self.serial_conn.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        response_lines.append(line)
                        self.log_message(f"RECV: {line}")
                        
                        # Check for specific error patterns
                        self.analyze_response_line(line)
                        
            except Exception as e:
                print(f"Error reading response: {e}")
                break
            
            time.sleep(0.01)  # Small delay to prevent CPU overload
        
        return response_lines
    
    def analyze_response_line(self, line):
        """Analyze a response line for error patterns"""
        # Check for "Core obliterated Data" errors
        if "Core obliterated Data" in line:
            self.stats['core_obliterated_errors'] += 1
            self.log_message(f"ERROR DETECTED: Core obliterated Data - Line: {line}")
        
        # Check for SD card errors
        if "SD" in line and ("error" in line.lower() or "fail" in line.lower()):
            self.stats['sd_card_errors'] += 1
            self.log_message(f"ERROR DETECTED: SD Card Error - Line: {line}")
        
        # Check for audio dropouts
        if "dropout" in line.lower() or "underflow" in line.lower() or "overflow" in line.lower():
            self.stats['audio_dropouts'] += 1
            self.log_message(f"ERROR DETECTED: Audio Dropout - Line: {line}")
        
        # Check for FR_DISK_ERR
        if "FR_DISK_ERR" in line or "文件读取失败" in line:
            self.stats['sd_card_errors'] += 1
            self.log_message(f"ERROR DETECTED: FR_DISK_ERR - Line: {line}")
    
    def log_message(self, message):
        """Log a message with timestamp"""
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
        log_entry = f"[{timestamp}] {message}"
        print(log_entry)
        
        if self.log_file:
            self.log_file.write(log_entry + "\n")
            self.log_file.flush()
    
    def run_basic_system_test(self):
        """Run basic system functionality test"""
        print("Starting basic system test...")
        
        test_commands = [
            ("Query task resource", "Check system tasks"),
            ("Query task time", "Check task timing"),
            ("sd read", "Test SD card read"),
            ("music start", "Test audio playback start"),
            ("music stop", "Test audio playback stop"),
        ]
        
        for command, description in test_commands:
            print(f"\nTesting: {description}")
            self.send_command(command)
            response = self.read_response(timeout=10)
            
            # Analyze response for success/failure
            success = self.analyze_test_response(response, command)
            
            test_result = {
                'command': command,
                'description': description,
                'success': success,
                'response': response,
                'timestamp': datetime.datetime.now().isoformat()
            }
            
            self.test_results.append(test_result)
            self.stats['total_tests'] += 1
            
            if success:
                self.stats['passed_tests'] += 1
                print(f"✓ Test passed: {description}")
            else:
                self.stats['failed_tests'] += 1
                print(f"✗ Test failed: {description}")
            
            time.sleep(2)  # Wait between tests
    
    def analyze_test_response(self, response, command):
        """Analyze test response to determine success/failure"""
        if not response:
            return False
        
        response_text = " ".join(response).lower()
        
        # Check for common failure indicators
        failure_indicators = [
            "error", "fail", "timeout", "exception", "abort",
            "FR_DISK_ERR", "obliterated", "无法", "失败"
        ]
        
        for indicator in failure_indicators:
            if indicator in response_text:
                return False
        
        # Check for success indicators based on command
        if "task" in command.lower():
            return any("task" in line.lower() for line in response)
        elif "sd" in command.lower():
            return any("success" in line.lower() or "成功" in line for line in response)
        elif "music" in command.lower():
            return any("start" in line.lower() or "stop" in line.lower() or "开始" in line or "停止" in line for line in response)
        
        return True  # Default to success if no failures detected
    
    def run_stress_test(self, duration_minutes=10):
        """Run stress test for specified duration"""
        print(f"Starting stress test for {duration_minutes} minutes...")
        
        start_time = time.time()
        end_time = start_time + (duration_minutes * 60)
        
        test_cycle = 0
        
        while time.time() < end_time:
            test_cycle += 1
            print(f"\nStress test cycle {test_cycle}")
            
            # Test sequence
            commands = [
                "music start",
                "sd read",
                "Query task resource",
                "music stop"
            ]
            
            for command in commands:
                self.send_command(command)
                response = self.read_response(timeout=5)
                time.sleep(1)
            
            # Wait before next cycle
            time.sleep(10)
        
        print(f"Stress test completed after {test_cycle} cycles")
    
    def run_audio_quality_test(self):
        """Run audio quality and stability test"""
        print("Starting audio quality test...")
        
        # Start audio playback
        self.send_command("music start")
        response = self.read_response(timeout=5)
        
        if not self.analyze_test_response(response, "music start"):
            print("Failed to start audio playback")
            return False
        
        # Monitor audio for specified duration
        monitor_duration = 60  # 1 minute
        print(f"Monitoring audio quality for {monitor_duration} seconds...")
        
        start_time = time.time()
        audio_errors = 0
        
        while time.time() - start_time < monitor_duration:
            response = self.read_response(timeout=1)
            if response:
                for line in response:
                    if any(error in line.lower() for error in ["dropout", "underflow", "overflow", "obliterated"]):
                        audio_errors += 1
            
            time.sleep(1)
        
        # Stop audio playback
        self.send_command("music stop")
        self.read_response(timeout=5)
        
        print(f"Audio quality test completed. Errors detected: {audio_errors}")
        return audio_errors == 0
    
    def generate_report(self):
        """Generate comprehensive test report"""
        report = {
            'test_summary': {
                'total_tests': self.stats['total_tests'],
                'passed_tests': self.stats['passed_tests'],
                'failed_tests': self.stats['failed_tests'],
                'pass_rate': (self.stats['passed_tests'] / self.stats['total_tests'] * 100) if self.stats['total_tests'] > 0 else 0,
                'test_duration': (self.stats['test_end_time'] - self.stats['test_start_time']).total_seconds() if self.stats['test_end_time'] and self.stats['test_start_time'] else 0
            },
            'error_statistics': {
                'core_obliterated_errors': self.stats['core_obliterated_errors'],
                'sd_card_errors': self.stats['sd_card_errors'],
                'audio_dropouts': self.stats['audio_dropouts']
            },
            'test_results': self.test_results,
            'recommendations': []
        }
        
        # Add recommendations based on results
        if self.stats['core_obliterated_errors'] > 0:
            report['recommendations'].append("Implement task notification overflow protection")
            report['recommendations'].append("Review DMA interrupt timing and task priorities")
        
        if self.stats['sd_card_errors'] > 0:
            report['recommendations'].append("Implement SD card error recovery mechanisms")
            report['recommendations'].append("Review SD card SPI timing configuration")
        
        if self.stats['audio_dropouts'] > 0:
            report['recommendations'].append("Increase audio buffer size")
            report['recommendations'].append("Optimize audio processing task priority")
        
        return report
    
    def save_report(self, filename):
        """Save test report to file"""
        report = self.generate_report()
        
        with open(filename, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"Test report saved to {filename}")
    
    def print_summary(self):
        """Print test summary"""
        print("\n" + "="*50)
        print("BETA TEST SUMMARY")
        print("="*50)
        print(f"Total Tests: {self.stats['total_tests']}")
        print(f"Passed: {self.stats['passed_tests']}")
        print(f"Failed: {self.stats['failed_tests']}")
        
        if self.stats['total_tests'] > 0:
            pass_rate = (self.stats['passed_tests'] / self.stats['total_tests']) * 100
            print(f"Pass Rate: {pass_rate:.1f}%")
        
        print(f"\nError Statistics:")
        print(f"Core Obliterated Errors: {self.stats['core_obliterated_errors']}")
        print(f"SD Card Errors: {self.stats['sd_card_errors']}")
        print(f"Audio Dropouts: {self.stats['audio_dropouts']}")
        
        print("\nRecommendations:")
        if self.stats['core_obliterated_errors'] > 0:
            print("- Implement task notification overflow protection")
        if self.stats['sd_card_errors'] > 0:
            print("- Implement SD card error recovery mechanisms")
        if self.stats['audio_dropouts'] > 0:
            print("- Optimize audio buffer management")
        
        print("="*50)

def main():
    parser = argparse.ArgumentParser(description='BLE Audio Beta Test Automation')
    parser.add_argument('--port', '-p', required=True, help='Serial port (e.g., COM3, /dev/ttyUSB0)')
    parser.add_argument('--baud', '-b', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--test-type', '-t', choices=['basic', 'stress', 'audio', 'full'], 
                        default='basic', help='Test type to run')
    parser.add_argument('--duration', '-d', type=int, default=10, 
                        help='Test duration in minutes (for stress test)')
    parser.add_argument('--output', '-o', help='Output file for test report')
    parser.add_argument('--log', '-l', help='Log file for detailed output')
    
    args = parser.parse_args()
    
    # Create tester instance
    tester = BLEAudioTester(args.port, args.baud)
    
    # Set up logging
    if args.log:
        tester.log_file = open(args.log, 'w')
        print(f"Logging to {args.log}")
    
    # Connect to device
    if not tester.connect():
        sys.exit(1)
    
    try:
        tester.stats['test_start_time'] = datetime.datetime.now()
        
        # Run selected test type
        if args.test_type == 'basic':
            tester.run_basic_system_test()
        elif args.test_type == 'stress':
            tester.run_stress_test(args.duration)
        elif args.test_type == 'audio':
            tester.run_audio_quality_test()
        elif args.test_type == 'full':
            tester.run_basic_system_test()
            tester.run_audio_quality_test()
            tester.run_stress_test(args.duration)
        
        tester.stats['test_end_time'] = datetime.datetime.now()
        
        # Print summary
        tester.print_summary()
        
        # Save report if requested
        if args.output:
            tester.save_report(args.output)
        
    except KeyboardInterrupt:
        print("\nTest interrupted by user")
    except Exception as e:
        print(f"Test failed with error: {e}")
    finally:
        tester.disconnect()
        if tester.log_file:
            tester.log_file.close()

if __name__ == "__main__":
    main()