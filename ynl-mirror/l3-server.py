#!/usr/bin/env python3

import os
import pprint
import json
import requests

from tools.net.ynl.lib import YnlFamily, Netlink

dpu = 'ekho-dpu'
port = '4321'
dests = {}
tables = set()

def init_requests():
    retries = requests.adapters.Retry(connect=5, backoff_factor=0.2)
    a = requests.adapters.HTTPAdapter(max_retries=retries)
    s = requests.Session()
    s.mount('http://', a)

def init():
    init_requests()

    r = requests.get(f"http://{dpu}:{port}/route")
    if r.status_code != 200:
        print(f"Failed to get routes from hw agent on {dpu}")
        return

    for entry in r.json():
        if 'rta-dst' in entry:
            table = entry['rtm-table']
            dst = entry['rta-dst']
            dst_len = entry['rtm-dst-len']

            prefix = f"{dst}/{str(dst_len)}"
            dests[prefix] = entry
            if table < 253:
                tables.update([table])


def route(msg, op):
    dst = msg['rta-dst']
    dst_len = msg['rtm-dst-len']

    prefix = f"{dst}/{str(dst_len)}"
    if prefix in dests:
        next_hop = dests[prefix]
        oif = next_hop['rta-oif']
        table = next_hop['rtm-table']

        other_tables = tables - set([table])
        for t in other_tables:
            print(f"ip route {op} {prefix} table {str(t)} nexthop dev {oif}")

            headers = {'Content-type': 'application/json'}
            data = {
                "rtm-family": 2,
                "rtm-protocol": 3,
                "rtm-table": t,
                "rtm-type": 1,
                "rtm-dst-len": dst_len,
                "rta-dst": dst,
                "rta-oif": oif
            }
            r = requests.post(f"http://{dpu}:{port}/route/{op}",
                              data=json.dumps(data), headers=headers)
            print(r.status_code)
    else:
        print(f"No existing dest for {prefix}")


def handle(event):
    name = event['name']
    msg = event['msg']

    if name == 'newroute':
        route(msg, 'add')
    elif name == 'delroute':
        route(msg, 'del')
    else:
        print(f"Unhandled event {name}")


def main():

    init()

    dir = os.path.dirname(__file__)
    ynl = YnlFamily(f"{dir}/Documentation/netlink/specs/rt_route.yaml")
    ynl.ntf_subscribe('rtnlgrp-ipv4-route')

    print("l3-server is running.")

    for event in ynl.check_ntf():
        #pprint.PrettyPrinter().pprint(event)
        handle(event)

if __name__ == "__main__":
    main()
