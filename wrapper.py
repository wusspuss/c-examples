from ctypes import *
media_receiver = cdll.LoadLibrary("./media_receiver.so")
media_receiver.test()
