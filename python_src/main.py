#!/usr/bin/env python3
import ctypes
import sys
import os
if sys.platform.startswith('win'):
    cmd_win = ctypes.windll.kernel32.GetConsoleWindow()
    if cmd_win != 0:
        ctypes.windll.user32.ShowWindow(cmd_win, 0)

import sys
_outSTD = sys.stdout
_outTMP = open(os.path.join('data', 'outTemp'), 'w')
sys.stdout = _outTMP

import os
import wx
from core import *
from pattern import patternBuilding
from utils import jsonCheck, hideConsole, showConsole, hook
from gui import mainFrame, configFrame
from time import time, sleep


class mainWindow(mainFrame):
    '''
    应用主页面
    '''

    def __init__(self, title):
        super(mainWindow, self).__init__(title)
        self.source = Source(self)
        self.thread = None
        self.configWin = None
        self.state = True
        # 载入ffmpeg coder
        self.source.ffmpeg()

    def Start(self, event):
        if self.thread == None:
            pass
        elif self.thread.is_alive():
            return
        else:
            pass
        self.source.wlog(self, 'Start command from user', 0)
        self.state = True
        self.gauge.SetValue(0)
        path = self.dirpath.GetValue()
        if '*' in path: path = (self.source.default_path, )
        self.thread = ffmpegThread(self, (path, self.source.source_path))
        self.thread.start()

    def On(self, event):
        if self.thread == None or not self.thread.is_alive(): return
        self.state = True
        self.source.wlog(self, 'Set running switch on', 0)
        self.logger.AppendText("继续合并......\n\n")

    def Off(self, event):
        if self.thread == None or not self.thread.is_alive(): return
        self.state = False
        self.source.wlog(self, 'Set running switch off', 0)
        self.logger.AppendText("合并已暂停\n\n")

    def Settings(self, event):
        if self.source.setting_active == True:
            self.configWin.Close()
            sleep(0.1)
        self.source.setting_active = True
        self.configWin = configWindow(self)
        self.configWin.Show()


    def PatternBuild(self, event):
        sys.stdout = _outSTD
        os.system('cls' if sys.platform.startswith('win') else 'clear')
        showConsole()
        path = self.dirpath.GetValue()
        if '*' in path: path = self.source.default_path
        pattern = patternBuilding(path, self.source.probe_path)
        if pattern != None:
            self.source.patternSave(pattern)
        input("\nPress 'enter' to exit ...")
        hideConsole()
        os.system('cls' if sys.platform.startswith('win') else 'clear')
        sys.stdout = _outTMP

    def JsonCheck(self, event):
        path = self.dirpath.GetValue()
        if '*' in path: path = self.source.default_path
        jsonChecking = jsonCheck(path)
        if jsonChecking.num == 0:
            self.Mes('', '无可修复的Json文件')
        else:
            ret = self.Mes('', '发现%s个异常Json文件，确认修复？\n（操作不可逆）' % jsonChecking.num, wx.YES_NO)
            if ret == wx.ID_YES:
                checked = jsonChecking.do()
                self.source.wlog(self, "Json checked on path '%s'(num: %s)" % (path, checked), 0)
                self.Mes('', '%s个Json文件已修复' % checked)

    
    def Tutorial(self, e):
        content = self.source.doc(1)
        dlg = wx.TextEntryDialog(self, "", u"使用说明", content, style= \
            wx.TE_MULTILINE | wx.OK)
        dlg.ShowModal()
        dlg.Destroy()

    def Question(self, e):
        content = self.source.doc(2)
        dlg = wx.TextEntryDialog(self, "", u"相关问题", content, style= \
            wx.TE_MULTILINE | wx.OK)
        dlg.ShowModal()
        dlg.Destroy()

    def Consult(self, e):
        content = self.source.doc(3)
        dlg = wx.MessageDialog(self, content, u"反馈", wx.OK)
        dlg.ShowModal()
        dlg.Destroy()

    def About(self, e):
        content = self.source.doc(4)
        dlg = wx.MessageDialog(self, content, u"软件信息", wx.OK)
        dlg.ShowModal()
        dlg.Destroy()

    def Log(self, e):
        self.source.rlog()

    def Quit(self, e):
        self.source.dump()
        self.source.wlog(self, 'Close command from user', 0)
        if e.IsCommandEvent():
            self.Close()
        else:
            e.Skip()

    def Mes(self, e, content, style=wx.OK):
        dlg = wx.MessageDialog(self, content, u"提示", style)
        ret = dlg.ShowModal()
        dlg.Destroy()
        return ret

    def Diropen(self, e):
        dlg = wx.DirDialog(self, "选择待扫描文件夹", \
                           style=wx.DD_DEFAULT_STYLE | wx.DD_NEW_DIR_BUTTON)
        if dlg.ShowModal() == wx.ID_OK:
            self.dirpath.SetValue(dlg.GetPath())
        dlg.Destroy()


class configWindow(configFrame):

    def __init__(self, parent):
        super(configWindow, self).__init__(parent)
        self.src = parent.source

        self.m_comboBox1.Append(self.src.patternCheck())
        last_pattern = self.src.config.get('config', 'lastPattern')
        if last_pattern != None and last_pattern != '':
            self.m_comboBox1.SetValue(last_pattern)
        if self.src.config.getint('config', 'danmu2ass'):
            self.m_checkBox1.SetValue(True)
            self.DisplayDanmuConfig(None)
        if self.src.config.getint('config', 'coverSave'):
            self.m_checkBox2.SetValue(True)
        if self.src.config.getint('config', 'ccDown'):
            self.m_checkBox3.SetValue(True)
        if self.src.config.getint('config', 'oneDir'):
            self.m_checkBox4.SetValue(True)
        if self.src.config.getint('config', 'videoNumber'):
            self.m_checkBox5.SetValue(True)

    def Closing( self, event ):
        self.src.setting_active = False
        event.Skip()

    def DisplayDanmuConfig(self, event):
        if event == None or event.GetEventObject().GetValue():
            self.AddControl()
            self.m_textCtrl1.SetValue(self.src.config.get('config', 'fontSize'))
            self.m_textCtrl3.SetValue(self.src.config.get('config', 'textOpacity'))
            self.m_textCtrl5.SetValue(self.src.config.get('config', 'reverseBlank'))
            self.m_textCtrl2.SetValue(self.src.config.get('config', 'durationMarquee'))
            self.m_textCtrl4.SetValue(self.src.config.get('config', 'durationStill'))
            if self.src.config.getint('config', 'isReduceComments'):
                self.m_checkBox6.SetValue(True)
        else:
            self.DelControl()

    def Save(self, event):
        pattern_name = self.m_comboBox1.GetValue()
        if pattern_name == '自动': pattern_name = ''
        self.src.config.set('config', 'lastPattern', pattern_name)
        self.src.pattern = self.src.patternLoad(pattern_name)

        self.src.config.set('config', 'danmu2ass', str(int(self.m_checkBox1.GetValue())))
        self.src.config.set('config', 'coverSave', str(int(self.m_checkBox2.GetValue())))
        self.src.config.set('config', 'ccDown', str(int(self.m_checkBox3.GetValue())))
        self.src.config.set('config', 'oneDir', str(int(self.m_checkBox4.GetValue())))
        self.src.config.set('config', 'videoNumber', str(int(self.m_checkBox5.GetValue())))

        if self.m_checkBox1.GetValue():
            try:
                self.src.config.set('config', 'fontSize', str(int(self.m_textCtrl1.GetValue())))
                value = float(self.m_textCtrl3.GetValue())
                if value > 1 or value <= 0: raise ValueError('value out of range: %s' % value)
                self.src.config.set('config', 'textOpacity', str(round(value, 2)))
                value = float(self.m_textCtrl5.GetValue())
                if value >= 1 or value < 0: raise ValueError('value out of range: %s' % value)
                self.src.config.set('config', 'reverseBlank', str(round(value, 2)))
                self.src.config.set('config', 'durationMarquee', str(int(self.m_textCtrl2.GetValue())))
                self.src.config.set('config', 'durationStill', str(int(self.m_textCtrl4.GetValue())))
                self.src.config.set('config', 'isReduceComments', str(int(self.m_checkBox6.GetValue())))
            except ValueError as e:
                e = str(e).split(': ')[-1]
                dlg = wx.MessageDialog(self, "存在非法数据：%s， 请检查后重新输入。" % e, u"警告", wx.OK)
                dlg.ShowModal()
                dlg.Destroy()
                return
        self.src.danmuConfigLoad()

        sleep(0.5)
        self.Close()

    def Quit(self, event):
        self.Close()


if __name__ == '__main__':
    app = wx.App(False)
    frame = mainWindow(__version__)
    app.MainLoop()


_outTMP.close()

