#!/usr/bin/env python3

import pprint
from flask import Flask, request, jsonify
from tools.net.ynl.lib import YnlFamily, Netlink

ynl = YnlFamily('Documentation/netlink/specs/rt_route.yaml')

app = Flask('ekho-hw-agent')
app.debug = True

status = 'init'

@app.get('/route')
def get_routes():
    routes = ynl.getroute({'rtm-family': 2}, [], dump=True)
    return jsonify(routes)

@app.post('/route/add')
def add_route():
    resp = ynl.newroute(request.json, [Netlink.NLM_F_CREATE])
    print(resp)
    return '', 201

@app.post('/route/del')
def del_route():
    resp = ynl.delroute(request.json, [])
    return '', 201

@app.route('/')
def get_status():
    return jsonify({ 'status': status })

def main():
    app.run(host='0.0.0.0', port=4321, threaded=True)

if __name__ == "__main__":
    main()
