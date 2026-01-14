# telemetry_registry.py
# used to track what charts are active and what messages are needed
from collections import defaultdict

registry       = defaultdict(list)   # chart_type -> [instances]
signal_router  = defaultdict(list)   # signal_name -> [instances]
