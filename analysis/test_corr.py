import scipy.signal as sig
import numpy as np
import pylab
import scipy

s2 = np.array( [0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0] )
s1 = np.array( [0,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0] )
s3 = np.array( [0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,0] )
l = len( s1 )

pylab.subplot( 311 )
pylab.plot( s1, label = 's1' )
pylab.plot( s2, label = 's2' )
pylab.plot( s3, label = 's3' )
pylab.legend( )

pylab.subplot( 312 )
c11 = sig.correlate( s1, s1, 'same' )
c12 = sig.correlate( s1, s2, 'same' )
c13 = sig.correlate( s1, s3, 'same' )
c23 = sig.correlate( s2, s3, 'same' )
c32 = sig.correlate( s3, s2, 'same' )

pylab.plot( c11, label = 's1 - s1' )
pylab.plot( c12, label = 's1 - s2' )
pylab.plot( c13, label = 's1 - s3' )
pylab.plot( c23, label = 's2 - s3' )
pylab.plot( c32, label = 's3 - s2' )
pylab.legend( )

pylab.subplot( 313 )
print np.argmax( c11 ) - l/2 
print np.argmax( c12 ) - l/2
print np.argmax( c13 ) - l/2
print np.argmax( c23 ) - l/2
print np.argmax( c32 ) - l/2


pylab.show( )
