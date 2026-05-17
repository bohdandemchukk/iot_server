#!/bin/bash

MQTT_HOST=$(echo $MQTT_URL | sed 's|ssl://||' | cut -d':' -f1)
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
        echo "   Got: $actual"
        ((FAIL++))
    fi
}

mosquitto_pub \
    -h "$MQTT_HOST" \
    -p 8883 \
    --cafile /etc/ssl/certs/ca-certificates.crt \
    -u "$MQTT_USERNAME" \
    -P "$MQTT_PASSWORD" \
    -t "project/weather" \
    -m "temperature=99.9,humidity=88.8,pressure=77.7"

sleep 2  

response=$(curl -s $BASE_URL/latest)
check "E2E: MQTT message appears in /latest" "99.9" "$response"

response=$(curl -s $BASE_URL/last_hour)
check "E2E: MQTT message appears in /last_hour" "99.9" "$response"

FROM=$(date -u -d '5 minutes ago' '+%Y-%m-%dT%H:%M:%SZ')
TO=$(date -u '+%Y-%m-%dT%H:%M:%SZ')

response=$(curl -s "$BASE_URL/range?from=$FROM&to=$TO")
check "E2E: MQTT message appears in /range" "99.9" "$response"

echo ""
echo "E2E Results: $PASS passed, $FAIL failed"
exit $FAIL
