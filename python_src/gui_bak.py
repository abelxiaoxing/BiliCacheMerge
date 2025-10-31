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
        menuCpb = menu1.Append(wx.ID_ANY, u'兼容/调试模式', u'以兼容模式开始合并')
        self.Bind(wx.EVT_MENU, self.Cpbmode, menuCpb)
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

    def Cpbmode(self, event):
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
		wx.Frame.__init__ ( self, parent, id = wx.ID_ANY, title = u"首选项", pos = wx.DefaultPosition, size = wx.Size( 560,580 ), style = wx.DEFAULT_FRAME_STYLE|wx.FRAME_FLOAT_ON_PARENT|wx.TAB_TRAVERSAL )

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetMinSize(wx.Size( 560,580 ))
		self.SetMaxSize(wx.Size( 560,580 ))

		self.SetIcon(images.SettingIco.GetIcon())

		bSizer16 = wx.BoxSizer( wx.VERTICAL )

		self.m_panel3 = wx.Panel( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		bSizer17 = wx.BoxSizer( wx.VERTICAL )

		sbSizer4 = wx.StaticBoxSizer( wx.StaticBox( self.m_panel3, wx.ID_ANY, u"搜索设置" ), wx.VERTICAL )

		bSizer20 = wx.BoxSizer( wx.HORIZONTAL )

		self.m_staticText10 = wx.StaticText( sbSizer4.GetStaticBox(), wx.ID_ANY, u"缓存客户端：", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText10.Wrap( -1 )

		self.m_staticText10.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

		bSizer20.Add( self.m_staticText10, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )

		m_comboBox2Choices = [ u"自动" ]
		self.m_comboBox2 = wx.ComboBox( sbSizer4.GetStaticBox(), wx.ID_ANY, u"自动", wx.DefaultPosition, wx.Size( 150,-1 ), m_comboBox2Choices, wx.CB_DROPDOWN|wx.CB_READONLY )
		self.m_comboBox2.SetSelection( 0 )
		bSizer20.Add( self.m_comboBox2, 0, wx.ALL, 5 )


		sbSizer4.Add( bSizer20, 0, wx.EXPAND|wx.TOP|wx.RIGHT|wx.LEFT, 5 )

		bSizer21 = wx.BoxSizer( wx.HORIZONTAL )

		self.m_staticText11 = wx.StaticText( sbSizer4.GetStaticBox(), wx.ID_ANY, u"    不同哔哩哔哩客户端缓存文件的文件结构不同。修改该项以指定搜索模式，如果预设缓存客户端都不支持你的视频文件，请从菜单栏 ”工具-搜索模式创建向导“ 来新建搜索模式。", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText11.Wrap( -1 )

		self.m_staticText11.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
		self.m_staticText11.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

		bSizer21.Add( self.m_staticText11, 1, wx.ALL|wx.EXPAND, 5 )


		sbSizer4.Add( bSizer21, 1, wx.EXPAND|wx.BOTTOM|wx.RIGHT|wx.LEFT, 5 )


		bSizer17.Add( sbSizer4, 2, wx.ALL|wx.EXPAND, 8 )

		sbSizer5 = wx.StaticBoxSizer( wx.StaticBox( self.m_panel3, wx.ID_ANY, u"合并设置" ), wx.VERTICAL )

		gSizer2 = wx.GridSizer( 2, 2, 0, 0 )

		bSizer22 = wx.BoxSizer( wx.VERTICAL )

		self.m_checkBox6 = wx.CheckBox( sbSizer5.GetStaticBox(), wx.ID_ANY, u"弹幕转换", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_checkBox6.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )
		self.m_checkBox6.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )

		bSizer22.Add( self.m_checkBox6, 0, wx.ALL, 5 )

		self.m_staticText12 = wx.StaticText( sbSizer5.GetStaticBox(), wx.ID_ANY, u"    开启/关闭xml格式弹幕缓存文件转ass格式（离线看视频时可作为字幕文件载入）。", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText12.Wrap( -1 )

		self.m_staticText12.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
		self.m_staticText12.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

		bSizer22.Add( self.m_staticText12, 1, wx.ALL|wx.EXPAND, 5 )


		gSizer2.Add( bSizer22, 1, wx.EXPAND, 5 )

		bSizer24 = wx.BoxSizer( wx.VERTICAL )

		self.m_checkBox7 = wx.CheckBox( sbSizer5.GetStaticBox(), wx.ID_ANY, u"分p编号", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_checkBox7.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

		bSizer24.Add( self.m_checkBox7, 0, wx.ALL, 5 )

		self.m_staticText13 = wx.StaticText( sbSizer5.GetStaticBox(), wx.ID_ANY, u"    开启后会在合并的分p视频名前加入编号。", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText13.Wrap( -1 )

		self.m_staticText13.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
		self.m_staticText13.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

		bSizer24.Add( self.m_staticText13, 1, wx.ALL|wx.EXPAND, 5 )


		gSizer2.Add( bSizer24, 1, wx.EXPAND, 5 )

		bSizer25 = wx.BoxSizer( wx.VERTICAL )

		self.m_checkBox8 = wx.CheckBox( sbSizer5.GetStaticBox(), wx.ID_ANY, u"保留/下载封面", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_checkBox8.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

		bSizer25.Add( self.m_checkBox8, 0, wx.ALL, 5 )

		self.m_staticText14 = wx.StaticText( sbSizer5.GetStaticBox(), wx.ID_ANY, u"    保留或联网下载视频的封面。", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText14.Wrap( -1 )

		self.m_staticText14.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
		self.m_staticText14.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

		bSizer25.Add( self.m_staticText14, 1, wx.ALL|wx.EXPAND, 5 )


		gSizer2.Add( bSizer25, 1, wx.EXPAND, 5 )

		bSizer26 = wx.BoxSizer( wx.VERTICAL )

		self.m_checkBox9 = wx.CheckBox( sbSizer5.GetStaticBox(), wx.ID_ANY, u"下载cc字幕", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_checkBox9.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, True, wx.EmptyString ) )

		bSizer26.Add( self.m_checkBox9, 0, wx.ALL, 5 )

		self.m_staticText15 = wx.StaticText( sbSizer5.GetStaticBox(), wx.ID_ANY, u"    如果视频有cc字幕，则自动下载至合并文件夹。", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText15.Wrap( -1 )

		self.m_staticText15.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_LIGHT, False, wx.EmptyString ) )
		self.m_staticText15.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INFOBK ) )

		bSizer26.Add( self.m_staticText15, 1, wx.ALL|wx.EXPAND, 5 )


		gSizer2.Add( bSizer26, 1, wx.EXPAND, 5 )


		sbSizer5.Add( gSizer2, 1, wx.ALL|wx.EXPAND, 5 )


		bSizer17.Add( sbSizer5, 3, wx.ALL|wx.EXPAND, 8 )

		self.bSizer18 = wx.BoxSizer( wx.VERTICAL )

		###
		blank_staticText = wx.StaticText( self.m_panel3, wx.ID_ANY, u"", wx.DefaultPosition, wx.DefaultSize, 0 )

		self.bSizer18.Add( blank_staticText, 0, wx.EXPAND, 5 )


		bSizer17.Add( self.bSizer18, 3, wx.ALL|wx.EXPAND, 8 )

		bSizer19 = wx.BoxSizer( wx.HORIZONTAL )

		self.m_button4 = wx.Button( self.m_panel3, wx.ID_ANY, u"保 存", wx.DefaultPosition, wx.Size( 126,42 ), 0 )
		self.m_button4.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )
		self.m_button4.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_ACTIVECAPTION ) )
		self.m_button4.SetDefault()

		bSizer19.Add( self.m_button4, 0, wx.ALL, 5 )

		self.m_button5 = wx.Button( self.m_panel3, wx.ID_ANY, u"取 消", wx.DefaultPosition, wx.Size( 120,40 ), 0 )
		self.m_button5.SetFont( wx.Font( 12, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, wx.EmptyString ) )

		bSizer19.Add( self.m_button5, 0, wx.ALL, 5 )


		bSizer17.Add( bSizer19, 0, wx.ALIGN_RIGHT|wx.ALL, 5 )


		self.m_panel3.SetSizer( bSizer17 )
		self.m_panel3.Layout()
		bSizer17.Fit( self.m_panel3 )
		bSizer16.Add( self.m_panel3, 1, wx.EXPAND |wx.ALL, 0 )


		self.SetSizer( bSizer16 )
		self.Layout()

		self.Centre( wx.BOTH )

		# Connect Events
		self.m_checkBox6.Bind( wx.EVT_CHECKBOX, self.DisplayDanmuConfig )
		self.m_button4.Bind( wx.EVT_BUTTON, self.Save )
		self.m_button5.Bind( wx.EVT_BUTTON, self.Quit )

	def AddControl( self ):
		sbSizer6 = wx.StaticBoxSizer( wx.StaticBox( self.m_panel3, wx.ID_ANY, u"弹幕设置" ), wx.HORIZONTAL )

		bSizer27 = wx.BoxSizer( wx.VERTICAL )

		bSizer29 = wx.BoxSizer( wx.HORIZONTAL )

		self.m_staticText16 = wx.StaticText( sbSizer6.GetStaticBox(), wx.ID_ANY, u"字体大小：", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText16.Wrap( -1 )

		bSizer29.Add( self.m_staticText16, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )

		self.m_textCtrl4 = wx.TextCtrl( sbSizer6.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer29.Add( self.m_textCtrl4, 0, wx.ALL, 5 )


		bSizer27.Add( bSizer29, 1, wx.EXPAND, 5 )

		bSizer30 = wx.BoxSizer( wx.HORIZONTAL )

		self.m_staticText17 = wx.StaticText( sbSizer6.GetStaticBox(), wx.ID_ANY, u"不透明度：", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText17.Wrap( -1 )

		bSizer30.Add( self.m_staticText17, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )

		self.m_textCtrl5 = wx.TextCtrl( sbSizer6.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer30.Add( self.m_textCtrl5, 0, wx.ALL, 5 )


		bSizer27.Add( bSizer30, 1, wx.EXPAND, 5 )

		bSizer31 = wx.BoxSizer( wx.HORIZONTAL )

		self.m_staticText18 = wx.StaticText( sbSizer6.GetStaticBox(), wx.ID_ANY, u"底部空白：", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText18.Wrap( -1 )

		bSizer31.Add( self.m_staticText18, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )

		self.m_textCtrl6 = wx.TextCtrl( sbSizer6.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer31.Add( self.m_textCtrl6, 0, wx.ALL, 5 )


		bSizer27.Add( bSizer31, 1, wx.EXPAND, 5 )


		sbSizer6.Add( bSizer27, 1, wx.EXPAND|wx.TOP|wx.BOTTOM|wx.LEFT, 5 )

		bSizer28 = wx.BoxSizer( wx.VERTICAL )

		bSizer32 = wx.BoxSizer( wx.HORIZONTAL )

		self.m_staticText19 = wx.StaticText( sbSizer6.GetStaticBox(), wx.ID_ANY, u"滚动时长：", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText19.Wrap( -1 )

		bSizer32.Add( self.m_staticText19, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )

		self.m_textCtrl7 = wx.TextCtrl( sbSizer6.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		bSizer32.Add( self.m_textCtrl7, 0, wx.ALL, 5 )


		bSizer28.Add( bSizer32, 1, wx.EXPAND, 5 )

		bSizer33 = wx.BoxSizer( wx.HORIZONTAL )

		self.m_staticText20 = wx.StaticText( sbSizer6.GetStaticBox(), wx.ID_ANY, u"悬停时长：", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText20.Wrap( -1 )

		bSizer33.Add( self.m_staticText20, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )

		self.m_textCtrl8 = wx.TextCtrl(sbSizer6.GetStaticBox(), wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0)
		bSizer33.Add(self.m_textCtrl8, 0, wx.ALL, 5)


		bSizer28.Add( bSizer33, 1, wx.EXPAND, 5 )

		self.m_checkBox10 = wx.CheckBox( sbSizer6.GetStaticBox(), wx.ID_ANY, u"删除重叠字幕：", wx.DefaultPosition, wx.DefaultSize, wx.ALIGN_RIGHT )
		bSizer28.Add( self.m_checkBox10, 0, wx.TOP|wx.BOTTOM|wx.RIGHT, 5 )


		sbSizer6.Add( bSizer28, 1, wx.EXPAND|wx.TOP|wx.BOTTOM|wx.RIGHT, 5 )


		self.bSizer18.Clear(True)
		
		self.bSizer18.Add( sbSizer6, 0, wx.EXPAND, 5 )
		
		self.Layout()

	def DelControl( self ):
		self.bSizer18.Clear(True)
		
		blank_staticText = wx.StaticText( self.m_panel3, wx.ID_ANY, u"", wx.DefaultPosition, wx.DefaultSize, 0 )

		self.bSizer18.Add( blank_staticText, 0, wx.EXPAND, 5 )

		self.Layout()

	def __del__( self ):
		pass


	# Virtual event handlers, overide them in your derived class
	def DisplayDanmuConfig( self, event ):
		event.Skip()
	
	def Save( self, event ):
		event.Skip()

	def Quit( self, event ):
		event.Skip()


