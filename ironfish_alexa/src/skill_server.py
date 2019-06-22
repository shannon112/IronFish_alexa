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
pub = rospy.Publisher('naviIntent', String, queue_size=1)
NGROK = rospy.get_param('/ngrok', None)


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
        return resp.task_flag
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e


@ask.intent('MadeFromIntent')
def made_intent_function():
    return statement("Hi, I am iron fish. From National Taiwan University, Department of Electrical Engineering, a.k.a. N.T.U.E.E.")


@ask.intent('NavigationIntent', default={'place':"", 'object':"", 'roomNumber':""})
def navi_intent_function(place, object, roomNumber):
    '''
    Executed when "TestIntent" is called:
    say "Alexa, ask tester to say (first name of a person)"
    Note that the 'intent_name' argument of the decorator @ask.intent
    must match the name of the intent in the Alexa skill.
    '''
    location = "default"
    object_ = "center"

    # prasing location
    if len(place)>0 and len(roomNumber)>0: location = place + roomNumber
    elif len(place)>0 and len(roomNumber)==0: location = place
    else: location = location

    # prasing object
    if len(object)>0:
        object_list = object.split(' ')
        if len(object_list)>1:
            temp = ""
            for i,item in enumerate(range(len(object_list)-1)):
                if len(object_list[i])>len(object_list[i+1]): temp=object_list[i]
                elif object_list[i] == "the": temp=object_list[i+1]
                elif object_list[i+1] == "the": temp=object_list[i]
                else: temp=object_list[i+1]
            object_ = temp
        else:
            object_ = object
    else: object_ = object_
    print(object_)
    # call service to navi and  pubish topic as tracing use
    output = "obj: {};  loc: {};  num: {}".format(object_,location,roomNumber)
    pub.publish(output)
    captain_resp = send_command(object_,location)
    print "service responese:", captain_resp

    if bool(captain_resp):
        if object_ == "center": return statement('Ok, I am on the way to the {} of {}.'.format(object_,location))
        else: return statement('Ok, I am on the way to the {} in {}.'.format(object_,location))
    else:
        if object_ == "center": return statement('Sorry, the place {} is not registed on the map.'.format(location))
        else: return statement('Sorry, the {} in {} is not registed on the map.'.format(object_,location))

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
