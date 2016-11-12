#!/usr/bin/python3

# Print today's date in JST.

from datetime import datetime, timedelta

print((datetime.utcnow() + timedelta(hours=9)).strftime("%Y%m%d"))
