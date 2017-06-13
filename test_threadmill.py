"""test_threadmill.py: 

Read the data from thread-mill and analyze it.

"""
from __future__ import print_function

    
__author__           = "Dilawar Singh"
__copyright__        = "Copyright 2017-, Dilawar Singh"
__version__          = "1.0.0"
__maintainer__       = "Dilawar Singh"
__email__            = "dilawars@ncbs.res.in"
__status__           = "Development"

import sys
import logging
import os
import numpy as np
import time
import datetime
import scipy.signal as sig
import gnuplotlib as gpl
sys.path.append( 'pyblink' )
import arduino

res_file_ = '__results__tmp__.csv' 

direc_ = 0
dir_ = [ ]

def stamp( ):
    return datetime.datetime.now().isoformat( )

def smooth( vec, N ):
    return np.convolve( vec, np.ones( N )/N, 'same' )

def line_to_data(line):
    """First element is timestamp, rest the data etc.
    """
    if '>' in line:
        return [ ]

    line = filter(None, line.split(',') )
    data = [ ]
    for x in line:
        try:
            data.append( int(x) )
        except ValueError as e:
            data.append( x )
    return data

def _readline( ser ):
    eol = b'\n'
    leneol = len(eol)
    line = bytearray()
    while True:
        c = ser.read(1)
        if c:
            line += c
            if line[-leneol:] == eol:
                break
        else:
            break

        if( len(line) > 100 ):
            break

    return bytes(line)

def compute_speed( tvec, yvec ):
    """Compute speed """
    ydiff = np.diff( yvec )
    minus1Ps = np.where( ydiff == -1 )
    minus1Ts = tvec[ minus1Ps ]
    tps = np.diff( minus1Ts )
    velocity = np.mean( 4.1 / tps )    # m / sec
    if np.isnan( velocity ):
        velocity = 0.0
    return velocity


def calculate_motion( t, s1, s2 ):
    """Caculate speed and direction of motion """
    global dir_
    global direc_
    t, s1, s2 = [ np.array( x ) for x in [ t, s1, s2] ]
    s1H = sig.hilbert( s1 )
    s2H = sig.hilbert( s2 )
    v1 = compute_speed( t, s1 )
    v2 = compute_speed( t, s2 )
    # gpl.plot( (t, s1), (t, s2), terminal = 'X11' )
    speed = (v1+v2)/2.0
    d = calculate_direction( t, s1, s2 )
    if speed > 0:
        dir_.append( d )
        if len( dir_ ) > 100:
            dir_.pop( 0 )
        mean, std = np.mean( dir_ ), np.std( dir_ )
        if d > mean + std:
            direc_ = 1
        elif d < mean - std:
            direc_ = -1
    else:
        direc_ = 0

    with open( res_file_, 'a' ) as f:
        f.write( '%g %g %g %g %g %g\n' % (t[-1], s1[-1], s2[-1], v1, v2, direc_ ) )

    return speed, direc_

def calculate_direction( t, s1, s2 ):
    """Infer direction. 
    By computing the correlation.
    """
    s1 = np.diff( s1 )
    s2 = np.diff( s2 )
    s1[ s1 < 0 ] = 0
    s2[ s2 < 0 ] = 0
    tvec, result = [ ], [ ]
    startN = 0
    startDetect = False
    for i, x in enumerate( s1 ):
        y = s2[ i ]
        if x == 1:
            if startN > 0:
                tvec.append( t[i] - t[startN] )
            startN = i
            startDetect = True
        if startDetect:
            if y == 1:
                startDetect = False
                result.append( i - startN )
    return 1000 * np.mean( result ) / np.mean( tvec )

def main( port, baud ):
    print( '[INFO] Using port %s baudrate %d' % (port, baud ) )
    tvec, motion1, motion2 = [ ], [ ], [ ]
    ar = arduino.ArduinoPort( port )
    ar.open( )
    speed, direction = 0.0, 0
    with open( res_file_, 'w' ) as f:
        f.write( "time s1 s2 v1 v2 dir\n" )

    while True:
        line = ar.read_line( )
        data = line_to_data( line )
        if len( data ) < 7:
            continue

        tvec.append( data[0] )
        motion1.append( data[5] )
        motion2.append( data[6] )
        try:
            N = 50
            speed,direction = calculate_motion( tvec[-N:], motion1[-N:], motion2[-N:] )
        except Exception as e:
            speed = 0.0
            direction = 0
            print( e )
        if speed > 0:
            print( '%.5f %s' % (speed, direction))

if __name__ == '__main__':
    port = sys.argv[1]
    baud = 38400
    main( port, baud )
