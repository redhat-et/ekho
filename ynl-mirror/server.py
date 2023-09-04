import pprint
import json

from tools.net.ynl.lib import YnlFamily, Netlink

def main():

    ynl = YnlFamily('Documentation/netlink/specs/rt_route.yaml')
    ynl.ntf_subscribe('rtnlgrp-ipv4-route')

    for msg in ynl.check_ntf():
        pprint.PrettyPrinter().pprint(msg)

if __name__ == "__main__":
    main()
