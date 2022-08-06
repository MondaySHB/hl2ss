#------------------------------------------------------------------------------
# This script receives encoded video from one of the HoloLens sideview
# grayscale cameras and plays it. The camera resolution is 640x480 30 FPS. The
# stream supports three operating modes: 0) video, 1) video + rig pose,
# 2) query calibration (single transfer).
#------------------------------------------------------------------------------

import hl2ss
import cv2
import av

# Settings --------------------------------------------------------------------

# HoloLens address
host = "192.168.1.15"

# Port
# Options:
# hl2ss.StreamPort.RM_VLC_LEFTFRONT
# hl2ss.StreamPort.RM_VLC_LEFTLEFT
# hl2ss.StreamPort.RM_VLC_RIGHTFRONT
# hl2ss.StreamPort.RM_VLC_RIGHTRIGHT
port = hl2ss.StreamPort.RM_VLC_LEFTFRONT

# Operating mode
# 0: video
# 1: video + rig pose
# 2: query calibration (single transfer)
mode = hl2ss.StreamMode.MODE_1

# Video encoding profile
profile = hl2ss.VideoProfile.H265_MAIN

# Encoded stream average bits per second
# Must be > 0
bitrate = 1*1024*1024

#------------------------------------------------------------------------------

if (mode == hl2ss.StreamMode.MODE_2):
    data = hl2ss.download_calibration_rm_vlc(host, port)
    print('Calibration data')
    print(data.uv2xy.shape)
    print(data.extrinsics)
    quit()

codec = av.CodecContext.create(hl2ss.get_video_codec_name(profile), 'r')
pose_printer = hl2ss.pose_printer(60)
fps_counter = hl2ss.framerate_counter(60)
glitch_detector = hl2ss.continuity_analyzer(hl2ss.TimeBase.HUNDREDS_OF_NANOSECONDS / hl2ss.Parameters_RM_VLC.FPS)
client = hl2ss.connect_client_rm_vlc(host, port, 1024, mode, profile, bitrate)

try:
    while True:
        data = client.get_next_packet()
        timestamp = data.timestamp
        for packet in codec.parse(data.payload):
            for frame in codec.decode(packet):
                image = frame.to_ndarray(format='bgr24')

                glitch_detector.push(timestamp)
                fps_counter.push()
                pose_printer.push(timestamp, data.pose)

                cv2.imshow('video', image)
                cv2.waitKey(1)
except:
    pass

client.close()