# import subprocess
import configparser
# import traceback
import sys
import os
# from os.path import join
from readprops import readProps
from platformio.project.helpers import get_project_dir

Import("env")

platform = env.PioPlatform()
config = env.GetProjectConfig()
board_1 = env["BOARD"]
board = board_1.upper()
print ("Board " + board)
subdirname = os.path.basename(os.path.dirname(config.get("platformio", "src_dir")))
print ("Project Name : " + subdirname)

prefsLoc = get_project_dir() + "\\wisblock\\version.properties"
verObj = readProps(prefsLoc)
# print("Renaming to " + projenv["PROGNAME"] + "_V_" + verObj['long'])

env.Replace(PROGNAME="../../../Generated/%s_V_%s_%s" % (subdirname, verObj['long'], board))
