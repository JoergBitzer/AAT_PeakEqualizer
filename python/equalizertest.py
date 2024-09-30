import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as signal

fs = 44100
f0 = 5000
Q = 5
dBgain = -12


A  = 10**(dBgain/40)   #  (for peaking and shelving EQ filters only)
w0 = 2*np.pi*f0/fs
alpha = np.sin(w0)/(2*Q) 

#            b0 =   1 + alpha*A
#            b1 =  -2*cos(w0)
#            b2 =   1 - alpha*A
#            a0 =   1 + alpha/A
#            a1 =  -2*cos(w0)
#            a2 =   1 - alpha/A

b = [1 + alpha*A, -2*np.cos(w0), 1 - alpha*A]
a = [1 + alpha/A, -2*np.cos(w0), 1 - alpha/A]
b = b/a[0]
a = a/a[0]
print(b)
print(a)


# delta impulse
fft_size = 2048
x = np.zeros(fft_size)
x[0] = 1

# filter
y = signal.lfilter(b, a, x)

# transfer function
H = np.fft.fft(y)

# frequency axis
f = np.linspace(0, fs/2, fft_size//2 + 1)
# plot
fig, ax = plt.subplots(1, 1)
ax.plot(f, 20*np.log10(np.abs(H[0:fft_size//2+1])))
plt.show()

