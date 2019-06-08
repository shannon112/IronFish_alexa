#!/usr/bin/env python
import os
import rospy
import threading
import numpy as np
import csv

from flask import Flask
from flask_ask import Ask, question, statement
from std_msgs.msg import String
from ironfish_captain.srv import captain_command

app = Flask(__name__)
ask = Ask(app, "/")

# ROS node, publisher, and parameter.
# The node is started in a separate thread to avoid conflicts with Flask.
# The parameter *disable_signals* must be set if node is not initialized
# in the main thread.

threading.Thread(target=lambda: rospy.init_node('test_node', disable_signals=True)).start()
pub = rospy.Publisher('test_pub', String, queue_size=1)
NGROK = rospy.get_param('/ngrok', None)

infoMap_path = rospy.get_param('infoMap_path',None)
"""
f = open(infoMap_path, 'r')
infoMap_csv = csv.reader(f, delimiter=',')
infoMap_list = list(infoMap_csv)[1:]
"""

@ask.launch
def launch():
    '''
    Executed when launching skill: say "Alexa, ask tester"
    '''
    welcome_sentence = 'Hello, this is ironfish, your personal robotics assistant.'
    return question(welcome_sentence)


def send_command(object_,location):
    try:
        command = rospy.ServiceProxy('captain_service', captain_command)
        resp = command(object_, location)
        print "service responese:", resp
        return resp
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e


@ask.intent('NavigationIntent', default={'place':"", 'object':"", 'roomNumber':""})
def test_intent_function(place, object, roomNumber):
    '''
    Executed when "TestIntent" is called:
    say "Alexa, ask tester to say (first name of a person)"
    Note that the 'intent_name' argument of the decorator @ask.intent
    must match the name of the intent in the Alexa skill.
    '''
    location = "default"
    object_ = "default"

    print place
    print object
    print roomNumber
    if len(place)>0 and len(roomNumber)>0: location = place + roomNumber
    elif len(place)>0 and len(roomNumber)==0: location = place
    else: location = location

    if len(object)>0: object_ = object
    else: object_ = object_

    output = object_+","+location
    pub.publish(output)
    send_command(object_,location)

    location = place +" "+ roomNumber +" "+ object
    if len(place)>0 and len(object)==0 and len(roomNumber)==0:
        return statement('Ok, I am on the way to {}.'.format(place))
    elif len(place)>0 and len(object)>0 and len(roomNumber)==0:
        return statement('Ok, I am on the way to {} at {}.'.format(object,place))
    elif len(place)>0 and len(object)==0 and len(roomNumber)>0:
        return statement('Ok, I am on the way to {} {}.'.format(place,roomNumber))
    elif len(place)>0 and len(object)>0 and len(roomNumber)>0:
        return statement('Ok, I am on the way to {} at {} {}.'.format(object,place,roomNumber))
    return statement('Ok, where I am ?')


@ask.intent('AMAZON.StopIntent')
def stop():
    return statement("Goodbye")


@ask.intent('AMAZON.CancelIntent')
def cancel():
    return statement("Goodbye")

@ask.intent('AMAZON.NavigateHomeIntent')
def home():
    return statement("Goodbye")

@ask.session_ended
def session_ended():
    return "{}", 200

@ask.session_ended
def session_ended():
    return "{}", 200


if __name__ == '__main__':
    if NGROK:
        print 'NGROK mode'
        app.run(host=os.environ['ROS_IP'], port=5000)
    else:
        print 'Manual tunneling mode'
        dirpath = os.path.dirname(__file__)
        cert_file = os.path.join(dirpath, '../config/ssl_keys/certificate.pem')
        pkey_file = os.path.join(dirpath, '../config/ssl_keys/private-key.pem')
        app.run(host=os.environ['ROS_IP'], port=5000,
                ssl_context=(cert_file, pkey_file))
