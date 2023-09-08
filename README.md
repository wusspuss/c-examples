libdatachannel conflicts with python somehow

media_receiver.c contains a translation of github.com/paullouisageneau/libdatachannel/blob/master/examples/media-receiver/main.cpp to pure C

Building:
`apt install libjson-glib-dev`
`make`

Usage:
```bash
# to display video
gst-launch-1.0 udpsrc address=127.0.0.1 port=5000 caps="application/x-rtp" ! queue ! rtph264depay ! video/x-h264,stream-format=byte-stream ! queue ! avdec_h264 ! queue ! autovideosink

./media_receiver
# open main.html
# copy json output to the web page and press Submit
# copy json output from the web page to media_receiver and press enter
# observe video from your webcam in a new window
```

Repeat the above steps replacing `./media_receiver` with `python wrapper.py`. It may work the first time - please retry, it may even take several times before it fails, but it will fail in different ways:
1. Sometimes the camera will turn off before you even copy the browser response to media_receiver
2. Sometimes it will stream for a bit and then turn off the camera and hang
3. Sometimes the camera will stay on but no video window will pop up - no datagrams will be reported as being received either

None of these happen no matter how many times I re-run the original media_receiver without Python. Initially I tried to use libdatachannel with python-cffi/ctypes directly and thought the Python callbacks were somehow intervening, but apparently it's just the Python interpreter being in the same process that leads to problems somehow.