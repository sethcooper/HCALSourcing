#!/usr/bin/env python

import string
import os
import sys
import subprocess


def GetNewCfgName(runNumber):
  return 'hcalSourceDataMon.'+str(runNumber)+'_cfg.py'

def GetPlotCfgName(runNumber):
  return 'hcalSourceDataMonPlots.'+str(runNumber)+'.py'

def GetNewNTupleFileName(runNumber):
  return 'hcalSourceDataMon.'+str(runNumber)+'.root'

def GetPlotFileName(runNumber):
  return 'hcalSourceDataMonPlots.'+str(runNumber)+'.root'

def GetFilenames(runNumber):
  fileList = ''
  oldDir = os.getcwd()
  os.chdir("/mnt/bigspool/usc")
  for files in os.listdir("."):
    if files.endswith(".root") and str(runNumber) in files:
      fileList+=("'file:/mnt/bigspool/usc/"+files+"',\n")
  os.chdir(oldDir)
  return fileList


def CreateNTupleConfigFile(runNumber):
  Path_Cfg = 'hcalsourcedatamonitor_template_cfg.py'
  newCfgName = GetNewCfgName(runNumber)
  config_file=open(Path_Cfg,'r')
  config_txt = config_file.read()
  config_file.close()
  # Replacements
  config_txt = config_txt.replace("XXX_TFILENAME_XXX", GetNewNTupleFileName(runNumber))
  config_txt = config_txt.replace("XXX_FILENAMES_XXX", GetFilenames(runNumber))
  # write
  config_file=open(str(runNumber)+'/'+newCfgName,'w')
  config_file.write(config_txt)
  config_file.close()


def CreatePlotsConfigFile(runNumber):
  Path_Cfg = 'hcalsourcedatamonplots_template.py'
  newCfgName = GetPlotCfgName(runNumber)
  config_file=open(Path_Cfg,'r')
  config_txt = config_file.read()
  config_file.close()
  # Replacements
  config_txt = config_txt.replace("XXX_NTUPLEFILE_XXX", GetNewNTupleFileName(runNumber))
  config_txt = config_txt.replace("XXX_PLOTFILE_XXX", GetPlotFileName(runNumber))
  # write
  config_file=open(str(runNumber)+'/'+newCfgName,'w')
  config_file.write(config_txt)
  config_file.close()


def CreateCfgs(runNumber):
  if not os.path.isdir(str(runNumber)):
    os.mkdir(str(runNumber))
  CreateNTupleConfigFile(runNumber)
  CreatePlotsConfigFile(runNumber)


# RUN
runsList = [
215590,
215594,
215595,    
215596,    
215597,  
215600,  
215602,    
215604,    
215607,    
215631,    
215632,
215659,    
215637,    
215649,
]

for run in runsList:
  print 'Run number:',run
  CreateCfgs(run)
  proc = subprocess.Popen(['cmsRun',GetNewCfgName(run)],cwd=os.getcwd()+'/'+str(run),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  out,err = proc.communicate()
  print out
  print err
  proc = subprocess.Popen(['hcalSourceDataMonPlots',GetPlotCfgName(run)],cwd=os.getcwd()+'/'+str(run),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  out,err = proc.communicate()
  print out
  print err
  





