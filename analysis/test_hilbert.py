import scipy.signal as sig
import numpy as np
import pylab
import scipy

s1 = np.array( [0,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0 ] )
s2 = np.array( [0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0 ] )
s3 = np.array( [0, 0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0 ] )
l = len( s1 )

s1 = np.interp( np.linspace(0, l, 100), np.arange( l ), s1 ) 
s2 = np.interp( np.linspace(0, l, 100), np.arange( l ), s2 ) 
s3 = np.interp( np.linspace(0, l, 100), np.arange( l ), s3 ) 

# N = len( s1 ) / 5
# s1 = np.convolve( s1, np.ones( N ) / N )
# s2 = np.convolve( s2, np.ones( N ) / N )

pylab.subplot( 311 )
pylab.plot( s1 )
pylab.plot( s2 )
pylab.plot( s3 )

pylab.subplot( 312 )
x1 = sig.hilbert( s1 )
x2 = sig.hilbert( s2 )
x3 = sig.hilbert( s3 )
pylab.plot( np.abs( x1 ) )
pylab.plot( np.abs( x2 ) )
pylab.plot( np.abs( x3 ) )

pylab.subplot( 313 )
p1 = np.unwrap( np.angle( x1 ) )
p2 = np.unwrap( np.angle( x2 ) )
p3 = np.unwrap( np.angle( x3 ) )

pylab.plot( p1 ) 
pylab.plot( p2 ) 
pylab.plot( p3 ) 




pylab.show( )
