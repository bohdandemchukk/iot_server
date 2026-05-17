#!/bin/bash

BASE_URL="http://localhost:8080"
PASS=0
FAIL=0

check() {
    local name=$1
    local expected=$2
    local actual=$3
    
    if echo "$actual" | grep -q "$expected"; then
        echo "✅ PASS: $name"
        ((PASS++))
    else
        echo "❌ FAIL: $name"
        echo "   Expected: $expected"
        echo "   Got: $actual"
        ((FAIL++))
    fi
}

response=$(curl -s $BASE_URL/latest)
check "GET /latest returns JSON" '"data"' "$response"

response=$(curl -s -o /dev/null -w "%{http_code}" $BASE_URL/latest)
check "GET /latest returns 200" "200" "$response"

response=$(curl -s $BASE_URL/last_hour)
check "GET /last_hour returns JSON array" "\[" "$response"

response=$(curl -s -o /dev/null -w "%{http_code}" $BASE_URL/last_hour)
check "GET /last_hour returns 200" "200" "$response"

response=$(curl -s -o /dev/null -w "%{http_code}" $BASE_URL/nonexistent)
check "GET /nonexistent returns 404" "404" "$response"

response=$(curl -s "$BASE_URL/range?from=2026-01-01T00:00:00Z&to=2026-05-30T00:00:00Z")
check "GET /range with valid dates returns JSON" "\[" "$response"

response=$(curl -s -o /dev/null -w "%{http_code}" \
    "$BASE_URL/range?from=2026-01-01T00:00:00Z&to=2026-05-30T00:00:00Z")
check "GET /range with valid dates returns 200" "200" "$response"

response=$(curl -s -o /dev/null -w "%{http_code}" \
    "$BASE_URL/range?from=2026-01T00:00:00Z&to=2026-05-30T00:00:00Z")
check "GET /range with invalid dates returns 400" "400" "$response"

response=$(curl -s -o /dev/null -w "%{http_code}" \
    "$BASE_URL/range?from=&to=")
check "GET /range with empty dates returns 400" "400" "$response"


echo ""
echo "Results: $PASS passed, $FAIL failed"
exit $FAIL
