#!/usr/bin/env python3

# Module containing base classes to build GUI of program 'm4sMerge_bili'.
# The event handlers should be overridden when subclassing.


import os
import wx
import wx.xrc
import images


class mainFrame(wx.Frame):
    '''
    构建应用GUI页面，包含多个直接被外部读取/写入的控件
    '''

    def __init__(self, title):
        wx.Frame.__init__(self, None, title=title)
        self.SetMinSize((400, 480))
        self.SetSize((500, 600))
        # 添加窗口图标
        self.SetIcon(images.AppIco.GetIcon())
        # 创建底部状态栏
        self.statusbar = self.CreateStatusBar()
        self.statusbar.SetFieldsCount(2)
        self.statusbar.SetStatusWidths([-1, -3])
        # 在底部状态栏嵌入进度条
        self.gauge = wx.Gauge(self.statusbar, range=100, style= \
            wx.GA_SMOOTH | wx.GA_HORIZONTAL)
        self.blank = TransparentStaticText(self.statusbar, size=(95, -1))
        box = wx.BoxSizer()
        box.Add(self.blank, proportion=1)
        box.Add(self.gauge, proportion=3, flag=wx.LEFT | wx.RIGHT | wx.ALIGN_CENTER_VERTICAL, border=5)
        self.statusbar.SetSizer(box)
        # 创建菜单栏
        menuBar = wx.MenuBar()
        self.SetMenuBar(menuBar)
        # 创建各个菜单项
        menu1 = wx.Menu()
        menuContinue = menu1.Append(wx.ID_YES, u'&继续\tF1', u'继续合并')
        self.Bind(wx.EVT_MENU, self.On, menuContinue)
        menu1.AppendSeparator()
        menuStop = menu1.Append(wx.ID_NO, u'&暂停\tF2', u'暂停合并')
        self.Bind(wx.EVT_MENU, self.Off, menuStop)
        menu1.AppendSeparator()
        self.errorskip = menu1.Append(wx.ID_ANY, u'跳过未知错误\tF3', u'遇到错误时是否中断合并', kind=wx.ITEM_CHECK)  # 是否跳过未知错误
        menu1.Check(self.errorskip.GetId(), False)
        menu1.AppendSeparator()
        menuSettings = menu1.Append(wx.ID_ANY, u'&设置...\tF5', u'打开软件设置')
        self.Bind(wx.EVT_MENU, self.Settings, menuSettings)
        menu1.AppendSeparator()
        menuExit = menu1.Append(wx.ID_EXIT, u'&退出\tF8', u'退出应用程序')
        self.Bind(wx.EVT_MENU, self.Quit, menuExit)

        menu2 = wx.Menu()
        menuTools1 = menu2.Append(wx.ID_ANY, u'搜索模式创建向导', u'新建搜索模式')
        self.Bind(wx.EVT_MENU, self.PatternBuild, menuTools1)
        menu2.AppendSeparator()
        menuTools2 = menu2.Append(wx.ID_ANY, u'json修复(不可逆，慎用)', u'删除多余的}')
        self.Bind(wx.EVT_MENU, self.JsonCheck, menuTools2)

        menu3 = wx.Menu()
        menuHelp1 = menu3.Append(wx.ID_HELP_PROCEDURES, u'使用说明', u'软件操作说明')
        self.Bind(wx.EVT_MENU, self.Tutorial, menuHelp1)
        menu3.AppendSeparator()
        menuHelp2 = menu3.Append(wx.ID_HELP_CONTENTS, u'相关问题', u'可能遇到的问题及解决方法')
        self.Bind(wx.EVT_MENU, self.Question, menuHelp2)
        menu3.AppendSeparator()
        menuHelp3 = menu3.Append(wx.ID_MORE, u'反馈', u'上报bug')
        self.Bind(wx.EVT_MENU, self.Consult, menuHelp3)

        menu4 = wx.Menu()
        menuStart = menu4.Append(wx.ID_ABOUT, u'软件信息', u'查看软件版本信息')
        self.Bind(wx.EVT_MENU, self.About, menuStart)
        menu4.AppendSeparator()
        menuContinue = menu4.Append(wx.ID_HELP_COMMANDS, u'日志', u'打开日志文件')
        self.Bind(wx.EVT_MENU, self.Log, menuContinue)

        menuBar.Append(menu1, u'选项')
        menuBar.Append(menu2, u'工具')
        menuBar.Append(menu3, u'帮助')
        menuBar.Append(menu4, u'关于')

        bkg = wx.Panel(self)
        # 仅有一行的文本输入框
        filelabel = wx.StaticText(bkg, -1, "选择路径：")
        self.dirpath = wx.TextCtrl(bkg, style=wx.TE_READONLY | wx.TE_RICH2)
        self.dirpath.SetDefaultStyle(wx.TextAttr('GREY'))
        self.dirpath.AppendText("*单击或拖拽，不填则搜索同目录下的待合并文件")
        drop_target = FileDrop(self.dirpath)
        self.dirpath.SetDropTarget(drop_target)
        self.dirpath.Bind(wx.EVT_LEFT_UP, self.Diropen)
        # 开始按钮
        btn = wx.Button(bkg, size=(-1, 45), label="开始合并")
        btn.SetDefault()
        btn.Bind(wx.EVT_BUTTON, self.Start)
        # 一个ComboBox控件（下拉菜单）
        modellabel = wx.StaticText(bkg, label="模式：")
        self.mode = wx.ComboBox(bkg, value='bilibili', choices=["bilibili", "通用"], \
                                size=(100, -1), style=wx.CB_DROPDOWN | wx.CB_READONLY)
        # 只读文本框(作为公用项，外部可直接修改)
        self.logger = wx.TextCtrl(bkg, style=wx.TE_MULTILINE | wx.HSCROLL | wx.TE_READONLY)
        # boxsizer布局控制
        hbox1 = wx.BoxSizer()
        hbox1.Add(filelabel, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        hbox1.Add(self.dirpath, proportion=1, flag=wx.LEFT | wx.EXPAND, border=0)
        hbox2 = wx.BoxSizer()
        hbox2.Add(modellabel, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL | wx.LEFT, border=0)
        hbox2.Add(self.mode, proportion=0, flag=wx.LEFT, border=5)
        hbox3 = wx.BoxSizer()
        hbox3.Add(wx.StaticText(bkg, label=""), proportion=0)
        vbox1 = wx.BoxSizer(wx.VERTICAL)
        vbox1.Add(hbox2, proportion=0, flag=wx.LEFT, border=15)
        vbox1.Add(hbox3, proportion=1, flag=wx.LEFT | wx.TOP, border=15)
        hbox4 = wx.BoxSizer()
        hbox4.Add(vbox1, proportion=2, flag=wx.ALIGN_CENTER_VERTICAL | wx.LEFT, border=0)
        hbox4.Add(btn, proportion=1, flag=wx.ALIGN_CENTER_VERTICAL | wx.RIGHT, border=25)
        vbox2 = wx.BoxSizer(wx.VERTICAL)
        vbox2.Add(hbox1, proportion=0, flag=wx.EXPAND | wx.ALL, border=5)
        vbox2.Add(hbox4, proportion=1, flag=wx.EXPAND | wx.ALL, border=5)
        vbox2.Add(self.logger, proportion=3, flag=wx.EXPAND | wx.ALL, border=5)
        bkg.SetSizer(vbox2)

        self.Bind(wx.EVT_CLOSE, self.Quit)

        # self.statusbar.SetAutoLayout(True)
        # box.Fit(self.statusbar)
        # 显示窗口
        self.Show(True)

    def Start(self, event):
        event.Skip()

    def On(self, event):
        event.Skip()

    def Off(self, event):
        event.Skip()

    def Settings(self, event):
        event.Skip()

    def PatternBuild(self, event):
        event.Skip()

    def JsonCheck(self, event):
        event.Skip()

    def Tutorial(self, event):
        event.Skip()

    def Question(self, event):
        event.Skip()

    def Consult(self, event):
        event.Skip()

    def About(self, event):
        event.Skip()

    def Log(self, event):
        event.Skip()

    def Quit(self, event):
        event.Skip()

    def Mes(self, event):
        event.Skip()

    def Diropen(self, event):
        event.Skip()


class FileDrop(wx.FileDropTarget):
    """
    实现文件拖拽
    """

    def __init__(self, target):
        wx.FileDropTarget.__init__(self)
        self.target = target

    def OnDropFiles(self, x, y, filenames):
        if len(filenames) == 1 and os.path.isdir(filenames[0]):
            path = filenames[0]
        else:
            path = os.path.dirname(filenames[0])
        self.target.SetValue(path)
        return True


class TransparentStaticText(wx.StaticText):
    """
    重写StaticText控件
    """

    def __init__(self, parent, id=wx.ID_ANY, label='', pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.TRANSPARENT_WINDOW, name='TransparentStaticText'):
        wx.StaticText.__init__(self, parent, id, label, pos, size, style, name)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_ERASE_BACKGROUND, lambda event: None)
        self.Bind(wx.EVT_SIZE, self.OnSize)

    def OnPaint(self, event):
        bdc = wx.PaintDC(self)
        dc = wx.GCDC(bdc)
        font_face = self.GetFont()
        font_color = self.GetForegroundColour()
        dc.SetFont(font_face)
        dc.SetTextForeground(font_color)
        dc.DrawText(self.GetLabel(), 0, 0)

    def OnSize(self, event):
        self.Refresh()
        event.Skip()


###########################################################################
## Python code below is generated with wxFormBuilder (version Oct 26 2018)
## http://www.wxformbuilder.org/
##
## Class configFrame
###########################################################################
class configFrame ( wx.Frame ):

    def __init__( self, parent ):
        wx.Frame.__init__ ( self, parent, id = wx.ID_ANY, title = u"首选项(仅bilibili模式)", pos = wx.DefaultPosition, size = wx.Size( 560,640 ), style = wx.DEFAULT_FRAME_STYLE|wx.FRAME_FLOAT_ON_PARENT|wx.TAB_TRAVERSAL )

        self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
        self.SetMinSize(wx.Size( 560,640 ))
        self.SetMaxSize(wx.Size( 560,640 ))

        self.SetIcon(images.SettingIco.GetIcon())

        bSizer1 = wx.BoxSizer( wx.VERTICAL )

        self.m_panel1 = wx.Panel( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
        bSizer2 = wx.BoxSizer( wx.VERTICAL )

        sbSizer1 = wx.StaticBoxSizer( wx.StaticBox( self.m_panel1, wx.ID_ANY, u" 搜索设置 " ), wx.VERTICAL )

        bSizer4 = wx.BoxSizer( wx.HORIZONTAL )

        self.m_staticText1 = wx.StaticText( sbSizer1.GetStaticBox(), wx.ID_ANY, u"缓存客户端：", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText1.Wrap( -1 )

        self.m_staticText1.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

        bSizer4.Add( self.m_staticText1, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )

        m_comboBox1Choices = [ u"自动" ]
        self.m_comboBox1 = wx.ComboBox( sbSizer1.GetStaticBox(), wx.ID_ANY, u"自动", wx.DefaultPosition, wx.Size( 150,-1 ), m_comboBox1Choices, wx.CB_DROPDOWN|wx.CB_READONLY )
        self.m_comboBox1.SetSelection( 0 )
        bSizer4.Add( self.m_comboBox1, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )


        sbSizer1.Add( bSizer4, 0, wx.EXPAND|wx.TOP|wx.RIGHT|wx.LEFT, 5 )

        bSizer5 = wx.BoxSizer( wx.HORIZONTAL )

        self.m_staticText2 = wx.StaticText( sbSizer1.GetStaticBox(), wx.ID_ANY, u"    不同哔哩哔哩客户端缓存文件的文件结构不同。修改该项以指定搜索模式，如果预设缓存客户端都不支持你的视频文件，请从菜单栏 ”工具-搜索模式创建向导“ 来新建搜索模式。", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText2.Wrap( -1 )

        self.m_staticText2.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
        self.m_staticText2.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

        bSizer5.Add( self.m_staticText2, 1, wx.ALL|wx.EXPAND, 5 )


        sbSizer1.Add( bSizer5, 1, wx.EXPAND|wx.BOTTOM|wx.RIGHT|wx.LEFT, 5 )


        bSizer2.Add( sbSizer1, 2, wx.ALL|wx.EXPAND, 8 )

        sbSizer2 = wx.StaticBoxSizer( wx.StaticBox( self.m_panel1, wx.ID_ANY, u" 合并设置 " ), wx.HORIZONTAL )

        gSizer1 = wx.GridSizer( 3, 1, 0, 0 )

        bSizer8 = wx.BoxSizer( wx.VERTICAL )

        self.m_checkBox1 = wx.CheckBox( sbSizer2.GetStaticBox(), wx.ID_ANY, u"弹幕转换", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_checkBox1.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

        bSizer8.Add( self.m_checkBox1, 0, wx.ALL, 5 )

        self.m_staticText3 = wx.StaticText( sbSizer2.GetStaticBox(), wx.ID_ANY, u"    开启/关闭xml格式弹幕缓存文件转ass格式\n（离线看视频时可作为字幕文件载入）。", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText3.Wrap( -1 )

        self.m_staticText3.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
        self.m_staticText3.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

        bSizer8.Add( self.m_staticText3, 1, wx.ALL|wx.EXPAND, 5 )


        gSizer1.Add( bSizer8, 1, wx.EXPAND, 5 )

        bSizer9 = wx.BoxSizer( wx.VERTICAL )

        self.m_checkBox2 = wx.CheckBox( sbSizer2.GetStaticBox(), wx.ID_ANY, u"保留/下载封面", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_checkBox2.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

        bSizer9.Add( self.m_checkBox2, 0, wx.ALL, 5 )

        self.m_staticText4 = wx.StaticText( sbSizer2.GetStaticBox(), wx.ID_ANY, u"    保留或联网下载视频的封面。", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText4.Wrap( -1 )

        self.m_staticText4.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
        self.m_staticText4.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

        bSizer9.Add( self.m_staticText4, 1, wx.ALL|wx.EXPAND, 5 )


        gSizer1.Add( bSizer9, 1, wx.EXPAND, 5 )

        bSizer10 = wx.BoxSizer( wx.VERTICAL )

        self.m_checkBox3 = wx.CheckBox( sbSizer2.GetStaticBox(), wx.ID_ANY, u"下载cc字幕", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_checkBox3.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

        bSizer10.Add( self.m_checkBox3, 0, wx.ALL, 5 )

        self.m_staticText5 = wx.StaticText( sbSizer2.GetStaticBox(), wx.ID_ANY, u"    如果视频有cc字幕，则自动下载至合并\n文件夹。", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText5.Wrap( -1 )

        self.m_staticText5.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
        self.m_staticText5.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

        bSizer10.Add( self.m_staticText5, 1, wx.ALL|wx.EXPAND, 5 )


        gSizer1.Add( bSizer10, 1, wx.EXPAND, 5 )


        sbSizer2.Add( gSizer1, 1, wx.ALL|wx.EXPAND, 5 )

        self.m_staticline1 = wx.StaticLine( sbSizer2.GetStaticBox(), wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.LI_VERTICAL )
        sbSizer2.Add( self.m_staticline1, 0, wx.EXPAND|wx.BOTTOM|wx.RIGHT, 5 )

        gSizer2 = wx.GridSizer( 3, 1, 0, 0 )

        bSizer11 = wx.BoxSizer( wx.VERTICAL )

        self.m_checkBox4 = wx.CheckBox( sbSizer2.GetStaticBox(), wx.ID_ANY, u"单文件夹输出", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_checkBox4.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

        bSizer11.Add( self.m_checkBox4, 0, wx.ALL, 5 )

        self.m_staticText6 = wx.StaticText( sbSizer2.GetStaticBox(), wx.ID_ANY, u"    默认关闭。开启后会将合并的视频导出至同一个文件夹内。", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText6.Wrap( -1 )

        self.m_staticText6.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
        self.m_staticText6.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

        bSizer11.Add( self.m_staticText6, 1, wx.ALL|wx.EXPAND, 5 )


        gSizer2.Add( bSizer11, 1, wx.EXPAND, 5 )

        bSizer12 = wx.BoxSizer( wx.VERTICAL )

        self.m_checkBox5 = wx.CheckBox( sbSizer2.GetStaticBox(), wx.ID_ANY, u"分p编号", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_checkBox5.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

        bSizer12.Add( self.m_checkBox5, 0, wx.ALL, 5 )

        self.m_staticText7 = wx.StaticText( sbSizer2.GetStaticBox(), wx.ID_ANY, u"    开启后会在分p视频名前加上编号。", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText7.Wrap( -1 )

        self.m_staticText7.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
        self.m_staticText7.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

        bSizer12.Add( self.m_staticText7, 1, wx.ALL|wx.EXPAND, 5 )


        gSizer2.Add( bSizer12, 1, wx.EXPAND, 5 )

        bSizer13 = wx.BoxSizer( wx.VERTICAL )


        gSizer2.Add( bSizer13, 1, wx.EXPAND, 5 )


        sbSizer2.Add( gSizer2, 1, wx.ALL|wx.EXPAND, 5 )


        bSizer2.Add( sbSizer2, 4, wx.ALL|wx.EXPAND, 8 )


        ###
        self.bSizer6 = wx.BoxSizer( wx.HORIZONTAL )

        blank_staticText = wx.StaticText( self.m_panel1, wx.ID_ANY, u"", wx.DefaultPosition, wx.DefaultSize, 0 )

        self.bSizer6.Add( blank_staticText, 0, wx.EXPAND, 5 )

        bSizer2.Add( self.bSizer6, 3, wx.ALL|wx.EXPAND, 8 )
        ###


        bSizer3 = wx.BoxSizer( wx.HORIZONTAL )

        self.m_button1 = wx.Button( self.m_panel1, wx.ID_ANY, u"保 存", wx.DefaultPosition, wx.Size( 126,42 ), 0 )

        self.m_button1.SetDefault()
        self.m_button1.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
        self.m_button1.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_ACTIVECAPTION ) )

        bSizer3.Add( self.m_button1, 0, wx.ALL, 5 )

        self.m_button2 = wx.Button( self.m_panel1, wx.ID_ANY, u"取 消", wx.DefaultPosition, wx.Size( 120,40 ), 0 )
        self.m_button2.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )

        bSizer3.Add( self.m_button2, 0, wx.ALL, 5 )


        bSizer2.Add( bSizer3, 0, wx.ALIGN_RIGHT|wx.ALL, 5 )


        self.m_panel1.SetSizer( bSizer2 )
        self.m_panel1.Layout()
        bSizer2.Fit( self.m_panel1 )
        bSizer1.Add( self.m_panel1, 1, wx.EXPAND, 5 )


        self.SetSizer( bSizer1 )
        self.Layout()

        self.Centre( wx.BOTH )

        # Connect Events
        self.Bind( wx.EVT_CLOSE, self.Closing )
        self.m_checkBox1.Bind( wx.EVT_CHECKBOX, self.DisplayDanmuConfig )
        self.m_button1.Bind( wx.EVT_BUTTON, self.Save )
        self.m_button2.Bind( wx.EVT_BUTTON, self.Quit )


    def AddControl( self ):

        sbSizer3 = wx.StaticBoxSizer( wx.StaticBox( self.m_panel1, wx.ID_ANY, u" 弹幕设置 " ), wx.HORIZONTAL )

        gSizer3 = wx.GridSizer( 3, 2, 0, 0 )

        bSizer22 = wx.BoxSizer( wx.HORIZONTAL )

        self.m_staticText8 = wx.StaticText( sbSizer3.GetStaticBox(), wx.ID_ANY, u"字体大小：", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText8.Wrap( -1 )

        bSizer22.Add( self.m_staticText8, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_textCtrl1 = wx.TextCtrl( sbSizer3.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        bSizer22.Add( self.m_textCtrl1, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )


        gSizer3.Add( bSizer22, 1, wx.EXPAND, 5 )

        bSizer23 = wx.BoxSizer( wx.HORIZONTAL )

        self.m_staticText9 = wx.StaticText( sbSizer3.GetStaticBox(), wx.ID_ANY, u"滚动时长：", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText9.Wrap( -1 )

        bSizer23.Add( self.m_staticText9, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_textCtrl2 = wx.TextCtrl( sbSizer3.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        bSizer23.Add( self.m_textCtrl2, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )


        gSizer3.Add( bSizer23, 1, wx.EXPAND, 5 )

        bSizer24 = wx.BoxSizer( wx.HORIZONTAL )

        self.m_staticText10 = wx.StaticText( sbSizer3.GetStaticBox(), wx.ID_ANY, u"不透明度：", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText10.Wrap( -1 )

        bSizer24.Add( self.m_staticText10, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_textCtrl3 = wx.TextCtrl( sbSizer3.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        bSizer24.Add( self.m_textCtrl3, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )


        gSizer3.Add( bSizer24, 1, wx.EXPAND, 5 )

        bSizer25 = wx.BoxSizer( wx.HORIZONTAL )

        self.m_staticText11 = wx.StaticText( sbSizer3.GetStaticBox(), wx.ID_ANY, u"悬停时长：", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText11.Wrap( -1 )

        bSizer25.Add( self.m_staticText11, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_textCtrl4 = wx.TextCtrl( sbSizer3.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        bSizer25.Add( self.m_textCtrl4, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )


        gSizer3.Add( bSizer25, 1, wx.EXPAND, 5 )

        bSizer26 = wx.BoxSizer( wx.HORIZONTAL )

        self.m_staticText12 = wx.StaticText( sbSizer3.GetStaticBox(), wx.ID_ANY, u"底部空白：", wx.DefaultPosition, wx.DefaultSize, 0 )
        self.m_staticText12.Wrap( -1 )

        bSizer26.Add( self.m_staticText12, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

        self.m_textCtrl5 = wx.TextCtrl( sbSizer3.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
        bSizer26.Add( self.m_textCtrl5, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )


        gSizer3.Add( bSizer26, 1, wx.EXPAND, 5 )

        bSizer27 = wx.BoxSizer( wx.HORIZONTAL )

        self.m_checkBox6 = wx.CheckBox( sbSizer3.GetStaticBox(), wx.ID_ANY, u"删除重叠字幕：", wx.DefaultPosition, wx.DefaultSize, wx.ALIGN_RIGHT )
        bSizer27.Add( self.m_checkBox6, 0, wx.ALIGN_CENTER_VERTICAL|wx.TOP|wx.BOTTOM|wx.RIGHT, 5 )


        gSizer3.Add( bSizer27, 1, wx.EXPAND, 5 )


        sbSizer3.Add( gSizer3, 1, wx.ALL|wx.EXPAND, 5 )


        self.bSizer6.Clear(True)

        self.bSizer6.Add( sbSizer3, 1, wx.EXPAND, 5 )

        self.Layout()


    def DelControl( self ):

        self.bSizer6.Clear(True)

        blank_staticText = wx.StaticText( self.m_panel1, wx.ID_ANY, u"", wx.DefaultPosition, wx.DefaultSize, 0 )

        self.bSizer6.Add( blank_staticText, 0, wx.EXPAND, 5 )

        self.Layout()


    def __del__( self ):
        pass


    # Virtual event handlers, overide them in your derived class
    def Closing( self, event ):
        event.Skip()

    def DisplayDanmuConfig( self, event ):
        event.Skip()

    def Save( self, event ):
        event.Skip()

    def Quit( self, event ):
        event.Skip()


