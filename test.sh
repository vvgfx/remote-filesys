#!/bin/bash

#
# test.sh / Practicum - 2
#
# Vishnu Vardan S / CS5600 / Northeastern University
# Fall 2025 / Dec 4, 2025
#

ROOT_DIR=$(pwd)
CLIENT_DIR="$ROOT_DIR/client"
SERVER_DIR="$ROOT_DIR/server"
SERVER_PID=""
PASS=0
FAIL=0

cleanup() {
    if [ -n "$SERVER_PID" ] && kill -0 $SERVER_PID 2>/dev/null; then
        kill -SIGINT $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    rm -rf "$SERVER_DIR/files/test"
    rm -f "$CLIENT_DIR/files/test/downloaded_"*
    rm -f "$CLIENT_DIR/files/test/version_"*
    rm -f "$CLIENT_DIR/files/test/concurrent_"*
    rm -f "$CLIENT_DIR/files/test/conflict.txt"
    rm -f "$CLIENT_DIR/files/test/to_delete.txt"
}

trap cleanup EXIT

echo ""
echo "Compiling"
echo "--------------------------------------"
cd "$CLIENT_DIR" && make clean && make
cd "$SERVER_DIR" && make clean && make
cd "$ROOT_DIR"

echo ""
echo "Setting up test files"
echo "--------------------------------------"
mkdir -p "$CLIENT_DIR/files/test"
mkdir -p "$SERVER_DIR/files/test"

echo "Whatever goes upon two legs is an enemy." > "$CLIENT_DIR/files/test/animal_farm.txt"
echo "No animal shall wear clothes." >> "$CLIENT_DIR/files/test/animal_farm.txt"
echo "No animal shall sleep in a bed." >> "$CLIENT_DIR/files/test/animal_farm.txt"
echo "No animal shall drink alcohol." >> "$CLIENT_DIR/files/test/animal_farm.txt"
echo "No animal shall kill any other animal." >> "$CLIENT_DIR/files/test/animal_farm.txt"
echo "All animals are equal." >> "$CLIENT_DIR/files/test/animal_farm.txt"

echo "Small test content" > "$CLIENT_DIR/files/test/small.txt"
touch "$CLIENT_DIR/files/test/empty.txt"

echo ""
echo "Test: Server not running"
echo "--------------------------------------"
cd "$CLIENT_DIR"

./rfs WRITE test/small.txt test/remote.txt
if [ $? -ne 0 ]; then echo "PASS: WRITE fails when server down"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs GET test/file.txt test/local.txt
if [ $? -ne 0 ]; then echo "PASS: GET fails when server down"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

echo ""
echo "Test: Invalid parameters"
echo "--------------------------------------"

./rfs
if [ $? -ne 0 ]; then echo "PASS: No arguments"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs UNKNOWN
if [ $? -ne 0 ]; then echo "PASS: Unknown command"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs WRITE
if [ $? -ne 0 ]; then echo "PASS: WRITE missing args"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs GET
if [ $? -ne 0 ]; then echo "PASS: GET missing args"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

echo ""
echo "Starting server"
echo "--------------------------------------"
cd "$SERVER_DIR"
./server &
SERVER_PID=$!
sleep 1
echo "Server PID: $SERVER_PID"

echo ""
echo "Test: WRITE"
echo "--------------------------------------"
cd "$CLIENT_DIR"

./rfs WRITE test/small.txt test/small.txt
if [ $? -eq 0 ]; then echo "PASS: WRITE small file"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs WRITE test/animal_farm.txt test/animal_farm.txt
if [ $? -eq 0 ]; then echo "PASS: WRITE animal_farm.txt"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs WRITE test/empty.txt test/empty.txt
if [ $? -eq 0 ]; then echo "PASS: WRITE empty file"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs WRITE test/small.txt test/small.txt
if [ $? -eq 0 ]; then echo "PASS: WRITE creates v2"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs WRITE test/nonexistent.txt test/remote.txt
if [ $? -ne 0 ]; then echo "PASS: WRITE nonexistent file fails"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

if [ -f "$SERVER_DIR/files/test/small.txt.v2" ]; then
    echo "PASS: Versioning works (v2 exists)"
    ((PASS++))
else
    echo "FAIL: v2 not found"
    ((FAIL++))
fi

echo ""
echo "Test: GET"
echo "--------------------------------------"

./rfs GET test/small.txt test/downloaded_small.txt
if [ $? -eq 0 ]; then echo "PASS: GET existing file"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs GET test/nonexistent.txt test/local.txt
if [ $? -ne 0 ]; then echo "PASS: GET nonexistent file fails"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

echo ""
echo "Test: LS"
echo "--------------------------------------"

./rfs LS test/small.txt
if [ $? -eq 0 ]; then echo "PASS: LS existing file"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs LS test/nonexistent.txt
if [ $? -ne 0 ]; then echo "PASS: LS nonexistent file fails"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

echo ""
echo "Test: GET_VERSION"
echo "--------------------------------------"

./rfs GET_VERSION test/small.txt v1 test/version_v1.txt
if [ $? -eq 0 ]; then echo "PASS: GET_VERSION v1"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs GET_VERSION test/small.txt v99 test/out.txt
if [ $? -ne 0 ]; then echo "PASS: GET_VERSION nonexistent version fails"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

echo ""
echo "Test: RM"
echo "--------------------------------------"

echo "delete me" > "files/test/to_delete.txt"
./rfs WRITE test/to_delete.txt test/to_delete.txt

./rfs RM test/to_delete.txt
if [ $? -eq 0 ]; then echo "PASS: RM existing file"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

./rfs RM test/nonexistent.txt
if [ $? -ne 0 ]; then echo "PASS: RM nonexistent file fails"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

mkdir -p "$SERVER_DIR/files/test/dir_to_delete"
echo "file1" > "$SERVER_DIR/files/test/dir_to_delete/file1.txt.v1"

./rfs RM test/dir_to_delete
if [ $? -eq 0 ]; then echo "PASS: RM directory"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

echo ""
echo "Test: Concurrent writes"
echo "--------------------------------------"

for i in 1 2 3; do
    echo "Concurrent content $i" > "files/test/concurrent_$i.txt"
done

./rfs WRITE test/concurrent_1.txt test/concurrent_1.txt &
pid1=$!
./rfs WRITE test/concurrent_2.txt test/concurrent_2.txt &
pid2=$!
./rfs WRITE test/concurrent_3.txt test/concurrent_3.txt &
pid3=$!
wait $pid1 $pid2 $pid3

count=$(ls "$SERVER_DIR/files/test/" 2>/dev/null | grep "concurrent_.*\.v1" | wc -l)
if [ "$count" -eq 3 ]; then
    echo "PASS: All 3 concurrent writes succeeded"
    ((PASS++))
else
    echo "FAIL: Only $count/3 concurrent writes"
    ((FAIL++))
fi

echo ""
echo "Test: Write conflict (same file)"
echo "--------------------------------------"

echo "conflict content" > "files/test/conflict.txt"

./rfs WRITE test/conflict.txt test/conflict_target.txt &
pid1=$!
./rfs WRITE test/conflict.txt test/conflict_target.txt &
pid2=$!
./rfs WRITE test/conflict.txt test/conflict_target.txt &
pid3=$!
wait $pid1 $pid2 $pid3

versions=$(ls "$SERVER_DIR/files/test/" 2>/dev/null | grep "conflict_target.txt.v" | wc -l)
if [ "$versions" -eq 3 ]; then
    echo "PASS: 3 versions created (locking works)"
    ((PASS++))
else
    echo "FAIL: Expected 3 versions, got $versions"
    ((FAIL++))
fi

rm -f files/test/concurrent_*.txt
rm -f files/test/conflict.txt

echo ""
echo "Test: STOP"
echo "--------------------------------------"

./rfs STOP
if [ $? -eq 0 ]; then echo "PASS: STOP command"; ((PASS++)); else echo "FAIL"; ((FAIL++)); fi

sleep 1

# Check if server actually stopped
if kill -0 $SERVER_PID 2>/dev/null; then
    echo "FAIL: Server still running"
    ((FAIL++))
else
    echo "PASS: Server stopped"
    ((PASS++))
fi

SERVER_PID=""

echo ""
echo "Summary"
echo "--------------------------------------"
echo "Passed: $PASS"
echo "Failed: $FAIL"

cd "$ROOT_DIR"