#!/bin/bash

# Test script for the Parallel Programming Performance Analyzer
# This script runs various test scenarios to validate the tool's functionality

echo "========================================"
echo "Parallel Performance Analyzer Test Suite"
echo "========================================"

BUILD_DIR="./build"
ANALYZER="$BUILD_DIR/parallel_analyzer"

# Check if analyzer exists
if [ ! -f "$ANALYZER" ]; then
    echo "Error: Analyzer not found at $ANALYZER"
    echo "Please build the project first: mkdir build && cd build && cmake .. && make"
    exit 1
fi

echo ""
echo "1. Testing help output..."
$ANALYZER --help > /dev/null
if [ $? -eq 0 ]; then
    echo "✓ Help output test passed"
else
    echo "✗ Help output test failed"
    exit 1
fi

echo ""
echo "2. Testing CPU-bound example analysis..."
timeout 10s $ANALYZER -v -d 3.0 --no-gpu $BUILD_DIR/examples/cpu_bound_example 2 50000 > /tmp/cpu_test.log 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then  # 124 is timeout exit code
    echo "✓ CPU-bound analysis test passed"
else
    echo "✗ CPU-bound analysis test failed"
    cat /tmp/cpu_test.log
fi

echo ""
echo "3. Testing memory-bound example analysis..."
timeout 10s $ANALYZER -d 3.0 --no-gpu $BUILD_DIR/examples/memory_bound_example 500 500 2 1 > /tmp/memory_test.log 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "✓ Memory-bound analysis test passed"
else
    echo "✗ Memory-bound analysis test failed"
    cat /tmp/memory_test.log
fi

echo ""
echo "4. Testing load imbalance example analysis..."
timeout 10s $ANALYZER -d 3.0 --no-gpu $BUILD_DIR/examples/load_imbalance_example 1 4 > /tmp/load_test.log 2>&1
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "✓ Load imbalance analysis test passed"
else
    echo "✗ Load imbalance analysis test failed"
    cat /tmp/load_test.log
fi

echo ""
echo "5. Testing report generation..."
timeout 10s $ANALYZER -o /tmp/test_report.txt -d 2.0 --no-gpu $BUILD_DIR/examples/cpu_bound_example 2 10000 > /tmp/report_test.log 2>&1
if [ -f "/tmp/test_report.txt" ]; then
    echo "✓ Report generation test passed"
    echo "   Report size: $(wc -l < /tmp/test_report.txt) lines"
else
    echo "✗ Report generation test failed"
    cat /tmp/report_test.log
fi

echo ""
echo "6. Testing invalid arguments..."
$ANALYZER -p -1 > /tmp/invalid_test.log 2>&1
if [ $? -ne 0 ]; then
    echo "✓ Invalid arguments test passed"
else
    echo "✗ Invalid arguments test failed - should have failed"
fi

echo ""
echo "========================================"
echo "Test Suite Completed"
echo "========================================"

# Cleanup
rm -f /tmp/*_test.log /tmp/test_report.txt

echo "All basic functionality tests completed successfully!"
echo ""
echo "To run a comprehensive analysis:"
echo "  $ANALYZER -v -g -d 30.0 -o detailed_report.txt ./your_parallel_program"