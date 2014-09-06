## Copyright (c) 2011-2012,
##  Jinseong Jeon <jsjeon@cs.umd.edu>
##  Jeff Foster   <jfoster@cs.umd.edu>
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## 1. Redistributions of source code must retain the above copyright notice,
## this list of conditions and the following disclaimer.
##
## 2. Redistributions in binary form must reproduce the above copyright notice,
## this list of conditions and the following disclaimer in the documentation
## and/or other materials provided with the distribution.
##
## 3. The names of the contributors may not be used to endorse or promote
## products derived from this software without specific prior written
## permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.

module ADB
  require 'timeout'

  ADBS = File.dirname(__FILE__)
  require "#{ADBS}/aapt"

  def ADB.restart
    system("adb kill-server")
    system("adb start-server")
  end

  @@acmd = "adb"

  @@lcat = @@acmd + " logcat"
  ACTM = "ActivityManager"
  @@altr = @@lcat + " -d #{ACTM}:D *:S"
  PKG = "umd.troyd"
  @@fltr = @@lcat + " -d #{PKG}:D *:S"

  RUN = " broadcast -a android.intent.action.RUN"
  @@am = @@acmd + " shell am"
  @@run = @@am + RUN

  def ADB.device(serial)
    @@acmd = "adb -s #{serial}"
    @@lcat = @@acmd + " logcat"
    @@altr = @@lcat + " -d #{ACTM}:D *:S"
    @@fltr = @@lcat + " -d #{PKG}:D *:S"
    @@am = @@acmd + " shell am"
    @@run = @@am + RUN
  end

  TO = 2

  def ADB.devices
    dv_cnt = 0
    em_cnt = 0
    dvs = `adb devices`
    dvs.each_line do |line|
      dv = line =~ /.+device$/
      dv_cnt = dv_cnt + 1 if dv != nil
      em = line =~ /emulator-\d+\s+device$/
      em_cnt = em_cnt + 1 if em != nil
    end
    return dv_cnt, em_cnt
  end

  def ADB.online?
    dv_cnt, em_cnt = ADB.devices
    return false if dv_cnt == 0
    if em_cnt > 0 
      begin
        Timeout.timeout(TO) do
          sync_logcat("", @@altr) != ""
        end
      rescue Timeout::Error
        false
      end
    else # means, real device!
      true
    end
  end

  SUCC = "Success"
  FAIL = "Failure"

  def ADB.uninstall(pkg=PKG)
    sync_msg("#{@@acmd} uninstall #{pkg}", [SUCC, FAIL])
  end

  def ADB.install(apk)
    sync_msg("#{@@acmd} install #{apk}", [SUCC])
  end

  def ADB.instAll(dir, cond)
    Dir.glob(dir + "/*.apk").each do |file|
      if file.downcase.include? cond
        ADB.uninstall(AAPT.pkg file)
        ADB.install file
      end
    end
  end

  def ADB.ignite(act)
    sync_logcat("#{@@am} startservice -n #{PKG}/.Ignite -e AUT #{act}", @@fltr)
  end

  def ADB.cmd(cmd, opts)
    ext = ""
    opts.each do |k, v|
      ext << " -e #{k} \"#{v}\""
    end
    sync_logcat("#{@@run} -e cmd #{cmd}#{ext}", @@fltr)
  end

private

  def ADB.sync_logcat(cmd, filter)
    out = ""
    system("#{@@lcat} -c")
    ADB.runcmd(cmd)
    while out == "" do
      sleep(TO)
      out = `#{filter}`
      sanitized = ""
      out.each_line do |line|
        sanitized += line if line.include? PKG
      end # device log is different
      out = sanitized
    end
    out
  end

  def ADB.sync_msg(cmd, msgs)
    out = ""
    while out == "" do
      out = ADB.runcmd(cmd)
      msgs.each do |msg|
        return msg if out.include? msg
      end
      out = ""
      sleep(TO)
    end
  end

  def ADB.runcmd(cmd)
    if cmd != nil and cmd != ""
      puts "shell$ #{cmd}" # to debug
      `#{cmd}`
    end
  end
end
