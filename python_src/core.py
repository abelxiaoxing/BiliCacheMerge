#!/usr/bin/env python3

# Module containing core classes of program 'm4sMerge_bili'.
# It's a pivotal part of the complete project.


import os
import sys
import json
import configparser
import subprocess
import threading
from threading import Thread

from inspect import stack
from traceback import format_exc
from time import time, ctime, sleep

from pattern import getByPath
from utils import copy_file, download
from danmaku2ass_light import Danmaku2ASS
import wx
import CCdownload


__all__ = ['__version__', 'Source', 'videoSearch', 'mainfunc', 'cmnfunc', 'ffmpegThread']
__version__ = "m4sMerge_bili v1.8"


class Source():
    '''
    绑定到一个MainWindow类实例的资源管理类
    '''

    def __init__(self, win):
        self.win = win
        self.setting_active = False

        # 获取脚本运行目录
        # self.base_path = sys.path[1]
        if getattr(sys, 'frozen', False):
            self.base_path = sys._MEIPASS
        else:
            self.base_path = os.path.dirname(os.path.abspath(__file__))

        # 跨平台可执行文件名处理
        if sys.platform.startswith('win'):
            ffmpeg_exe = 'ffmpeg.exe'
            ffprobe_exe = 'ffprobe.exe'
        else:
            ffmpeg_exe = 'ffmpeg'
            ffprobe_exe = 'ffprobe'

        self.ini_path = os.path.join(self.base_path, 'config.ini')
        self.config = configparser.ConfigParser()
        self.config.read(self.ini_path, encoding='GB18030')
        if self.config.getint('customPath', 'customPermission'):
            self.source_path = '"%s"' % os.fspath(self.config.get('customPath', 'ffmpegPath'))
            self.pattern_path = os.fspath(self.config.get('customPath', 'ffmpegPath'))
        else:
            self.source_path = '"%s"' % os.path.join(self.base_path, 'ffmpeg', ffmpeg_exe)
            self.pattern_path = os.path.join(os.path.dirname(self.base_path), 'pattern')
        self.probe_path = os.path.join(self.base_path, 'ffmpeg', ffprobe_exe)
        self.doc_path = os.path.join(self.base_path, 'data', 'doc.txt')
        # self.logger_path = os.path.join(os.getenv('TEMP'), 'm4sMerge_bili.log')
        self.logger_path = os.path.join(self.base_path, 'data', 'm4sMerge_bili.log')
        self.default_path = os.path.dirname(os.path.dirname(self.base_path))

        # 载入日志文件
        if not os.path.exists(self.logger_path):
            f = open(self.logger_path, 'w', encoding='utf-8')
            f.write('####%s\n' % self.logger_path)
            f.close()

        with open(self.logger_path, 'a', encoding='utf-8') as f:
            f.write(f'\n////{ctime()} @{__version__} Begins running\n')

        self.pattern = self.patternLoad(self.config.get('config', 'lastPattern'))
        self.danmu_config = None
        self.danmuConfigLoad()

        # 计算使用者等级
        using_time = self.config.getfloat('record', 'totalUsingTime')
        merged_video = self.config.getint('record', 'totalVideoNum')
        self.rank = int((merged_video/100)**0.3 + using_time/120) + 1
        self.launch_time = time()


    def ffmpeg(self):
        '''初始化ffmpeg.exe'''
        try:
            p = subprocess.Popen('%s -version' % self.source_path, shell=True, stdout= \
                subprocess.PIPE, stderr=subprocess.STDOUT, stdin= \
                                     subprocess.DEVNULL)
            p_out, p_err = p.communicate()
            text = str(p_out, encoding='utf-8').split('\r\n')[0]
            self.win.logger.AppendText(text + '\n\n')
            self.wlog(self, text, 0)
        except Exception as e:
            self.win.logger.AppendText('ffmpeg载入失败！\n')
            self.wlog(self, f'Fail to initailize ffmpeg: {repr(e)} Path: {self.source_path}', 1)
        # 额外内容:留言
        last_rank = self.config.getint('record', 'userRank')
        if self.rank != last_rank:
            if self.rank == 1:
                self.win.logger.AppendText('--程序：(｡･∀･)ﾉﾞ嗨~ 第一次见面呢，希望以后相处愉快呀！\n\n')
            if self.rank == 2:
                self.win.logger.AppendText('--程序：(/≧▽≦)/ 真的很高兴能帮到你，成为你的好伙伴......\n\n')
            if self.rank == 3:
                self.win.logger.AppendText('--程序：(/ω＼*) 和...和你在一起的时光，我是不会忘记的......\n\n')

    def patternLoad(self, name=None):
        '''载入模式文件（.pat）'''
        if name == None or name == '':
            patterns = {}
            for file in os.listdir(self.pattern_path):
                if file.split('.')[-1] == 'pat':
                    with open(os.path.join(self.pattern_path, file), encoding='utf-8') as data:
                        p = json.load(data)
                        try:
                            patterns[p['name']] = p
                        except KeyError:
                            pass
            return patterns
        else:
            try:
                with open(os.path.join(self.pattern_path, '%s.pat' % name), encoding='utf-8') as data:
                    p = json.load(data)
                return {p['name']: p}
            except (IOError, KeyError):
                pass

    def patternCheck(self):
        '''检索模式文件，返回所有模式文件的文件名'''
        patterns = []
        for file in os.listdir(self.pattern_path):
            if file.split('.')[-1] == 'pat':
                with open(os.path.join(self.pattern_path, file), encoding='utf-8') as data:
                    p = json.load(data)
                    try:
                        patterns.append(p['name'])
                    except KeyError:
                        pass
        return patterns

    def patternSave(self, *pattern):
        '''储存模式文件'''
        for p in pattern:
            with open(os.path.join(self.pattern_path, '%s.pat' % p['name']), 'w', encoding='utf-8') as f:
                json.dump(p, f)

    def danmuConfigLoad(self):
        config = []
        config.append(self.config.getint('config', 'fontSize'))
        config.append(self.config.getfloat('config', 'textOpacity'))
        config.append(self.config.getfloat('config', 'reverseBlank'))
        config.append(self.config.getint('config', 'durationMarquee'))
        config.append(self.config.getint('config', 'durationStill'))
        config.append(self.config.getint('config', 'isReduceComments'))
        self.danmu_config = config

    
    def doc(self, index=-1):
        '''读取帮助文档,返回文档中的指定内容'''
        with open(self.doc_path, 'r', encoding='utf-8') as f:
            doc = f.readlines()
        if index == 1:
            content = doc[22:26]
        elif index == 2:
            content = doc[28:]
        elif index == 3:
            content = doc[7:8]
        elif index == 4:
            content = doc[1:5]
        else:
            content = ''
        return '\n'.join(content)

    def wlog(self, it, content, index=-1):
        '''日志写入函数'''
        _class = it.__class__.__name__
        _func = stack()[1][3]
        if index == 0:
            head = 'INFO'
        elif index == 1:
            head = 'ERROR'
        elif index == 2:
            head = 'WARNNING'
        else:
            head = 'UNKOWN'
        with open(self.logger_path, 'a', encoding='utf-8') as f:
            f.write(f'{head}    [{_class}:{_func}][{ctime()}] {content}\n')

    def rlog(self):
        '''打开日志文件'''
        # 跨平台打开日志文件
        import platform
        if platform.system() == 'Windows':
            subprocess.Popen(f'start "" "{self.logger_path}"', shell=True, stdin=subprocess.DEVNULL)
        elif platform.system() == 'Darwin':  # macOS
            subprocess.Popen(f'open "{self.logger_path}"', shell=True, stdin=subprocess.DEVNULL)
        else:  # Linux and others
            subprocess.Popen(f'xdg-open "{self.logger_path}"', shell=True, stdin=subprocess.DEVNULL)

    def numRecord(self, num, which=0):
        if which == 0:
            self.config.set('record', 'totalVideoNum', str(int(num) + self.config.getint('record', 'totalVideoNum')))
        elif which == 1:
            self.config.set('record', 'totalGroupNum', str(int(num) + self.config.getint('record', 'totalGroupNum')))
        else:
            pass

    def dump(self):
        self.config.set('record', 'userRank', str(self.rank))
        self.config.set('record', 'totalUsingTime',
                        str(round((time() - self.launch_time)/60 + self.config.getfloat('record', 'totalUsingTime'), 2)))
        # 保存配置文件
        with open(self.ini_path, 'w', encoding='GB18030') as fw:
            fw.write(";config file of program 'm4sMerge_bili'\n;version: 1.0\n\n;DON'T edit unless you kown its effects\n\n")
            self.config.write(fw, False)


def videoSearch(path, pattern, _normalized=False):
    '''按给定搜索模式搜索指定目录下的缓存文件。注意：真实搜索的文件仅有标签文件，因此其他文件路径指向的文件有可能并不存在。

    :param path: path of directory to be searched
    :param pattern: dict of (pattern name: Pattern) couples or dict of a single Pattern
    :return: path tuple(pattern_name, entry, video, audio, danmu) or Group dict when having a group
    '''
    if not _normalized:
        import copy
        pattern = copy.deepcopy(pattern)
        if sys.platform != 'win32':
            patterns_to_check = []
            if 'name' in pattern:
                patterns_to_check.append(pattern)
            else:
                patterns_to_check.extend(pattern.values())
            
            for p_item in patterns_to_check:
                if 'search' in p_item and 'tree' in p_item['search'] and p_item['search']['tree']:
                    tree = p_item['search']['tree']
                    for key, value in tree.items():
                        if isinstance(value, str):
                            tree[key] = value.replace('\\', '/')
        _normalized = True

    def entryCheck(entry_path, pattern_name):
        '''按指定模式检查所给文件是否为entry文件'''
        get = pattern.get(pattern_name)
        if get == None:
            get = pattern
        if get == None:
            return False
        parse_section = get.get('parse', {})
        if parse_section.get('sid') != None:
            key = parse_section['sid']
        else:
            key = parse_section.get('aid')
        if key == None:
            return False
        try:
            with open(entry_path, 'r', encoding='utf-8') as data:
                if getByPath(json.load(data), key) != '':
                    return True
                else:
                    return False
        except (UnicodeDecodeError, json.decoder.JSONDecodeError):
            return False

    def nameParse(file_name, entry_path=None):
        '''返回非固定命名文件的文件名，仅用于非分组模式下的音视频文件和分组模式下的标签文件和弹幕文件'''
        cuts = file_name.split('%')
        index = 0
        while True:
            try:
                cut = cuts[index]
                if cut == 'group':
                    cuts[index] = dir_name
                elif cut == 'episode':
                    cuts[index] = entry.name
                else:
                    if entry_path != None:
                        with open(entry_path, 'r', encoding='utf-8') as data:
                            value = json.load(data).get(cut)
                        if value != None:
                            cuts[index] = str(value)
            except IndexError:
                break
            else:
                index += 1
        return ''.join(cuts)

    top = os.fspath(path)
    dir_name = os.path.basename(top)
    dirs = []

    try:
        scandir_it = os.scandir(top)
    except OSError:                                 # OSError
        return

    with scandir_it:
        while True:
            try:
                try:
                    entry = next(scandir_it)
                except StopIteration:
                    break
            except OSError:                         # OSError
                return

            if entry.is_dir():
                dirs.append(entry.path)

            else:
                # search for entry file of videos
                if 'name' in pattern:
                    p = {pattern['name']: pattern}
                    keys = [pattern['name']]
                else:
                    p = pattern
                    keys = list(pattern)
                for k in keys:
                    _p = p[k]['search']
                    if not _p['has_group']:
                        if entry.name == _p['name'] and entryCheck(entry.path, k):
                            dirs.clear()
                            join = os.path.join
                            # 确定是否为blv类型的缓存文件
                            blv_folders = [i for i in os.listdir(top) if i.split('.')[0] == 'lua']
                            # 如果搜索树中存在弹幕相对路径，则生成弹幕路径
                            if _p['tree']['d'] != None:
                                danmu_path = join(top, _p['tree']['d'])
                            else:
                                danmu_path = ''
                            if len(blv_folders) == 1:
                                blv_path = join(top, blv_folders[0])
                                yield k, \
                                      join(top, _p['tree']['e']), \
                                      blv_path, \
                                      'blv', \
                                      danmu_path
                            else:
                                video_entry = join(top, _p['tree']['e'])
                                yield k, \
                                      video_entry, \
                                      join(top, nameParse(_p['tree']['v'], video_entry)), \
                                      join(top, nameParse(_p['tree']['a'], video_entry)), \
                                      danmu_path
                    else:
                        if _p['name'][0] == '.':
                            entry_name = dir_name + _p['name']
                        else:
                            entry_name = _p['name']
                        if entry.name == entry_name and entryCheck(entry.path, k):
                            dirs.clear()
                            join = os.path.join
                            scandir_it = os.scandir(top)
                            body = []
                            group_entry = entry.path
                            while True:
                                try:
                                    entry = next(scandir_it)
                                except StopIteration:
                                    break
                                if entry.is_dir():
                                    video_entry = join(entry.path, nameParse(_p['tree']['e']))
                                    if not os.path.exists(video_entry): continue
                                    body.append((video_entry, \
                                                 join(entry.path, _p['tree']['v']), \
                                                 join(entry.path, _p['tree']['a']), \
                                                 join(entry.path, nameParse(_p['tree']['d'])), \
                                                 ))
                            # 如果搜索树中存在封面相对路径，则生成封面路径
                            if _p['tree']['c'] != None:
                                cover_path = join(top, _p['tree']['c'])
                            else:
                                cover_path = ''
                            group = {'pattern':k, \
                                     'entry':group_entry, \
                                     'body':body, \
                                     'cover':cover_path, \
                                     }
                            yield group

    for new_path in dirs:
        yield from videoSearch(new_path, pattern, _normalized=True)


class mainfunc():
    '''
    实例化时搜索所给路径下的可合并文件，生成一个可迭代的自律型对象
    每次迭代即创建一个ffmpeg子进程完成一次合并，返回值为0/1。
    '''

    def __init__(self, FilePath, CorePath, pattern, Danmu=False, Cover=False, Subtitle=False, \
                 ordered=False, one_dir=False, over_write=True, danmu_config=None):
        if isinstance(FilePath, tuple):
            self.path = FilePath[0]
            self.save_path = os.path.join(self.path, 'bili_merge')
        else:
            self.path = FilePath
            self.save_path = os.path.join(os.path.dirname(self.path), 'bili_merge')
        self.source_path = CorePath
        self.switch = Danmu
        self.switch2 = ordered
        self.switch3 = Subtitle
        self.switch4 = Cover
        self.switch5 = one_dir
        self.switch6 = over_write
        self.pattern = pattern
        self.danmu_config = danmu_config
        self.log = [0, ]
        self.mes = [0, ]

        self.num = 0
        self.index = 0
        self.failed = 0
        self.groups = self.groupSearch()
        self.group_names = list(self.groups)    # aid of all videos
        self.group_num = len(self.group_names)
        self.group = [None, None, None]    # [group-body, pattern-parse, group_info]
        self.count = 0    # number of videos of current group
        if self.group_num == 0:
            self.mes.append("未找到可合并文件，请检查路径或文件格式。")
            self.mes[0] += 1
            return
        for g in self.groups.values():
            self.num += len(g['body'])
        self.tabel = str.maketrans('', '', '''\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\
        \r\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"/:*?"<>| ''')
        os.makedirs(self.save_path, exist_ok=True)
        self.log.append("\n共检测到 %s 组共 %s 个待合并文件\n合并进程已开始，请耐心等待~~~\n" % (self.group_num, self.num))
        self.log[0] += 1

    def groupSearch(self):
        '''以视频组结构体的形式返回搜索到的待合并视频。

        structure Group demo:

        group = {'pattern': pattern_name_str,
                 'entry': group_entry_path_str,
                 'body': [(entry_path_str, video_path_str, audio_path_str, danmu_path_str), ...],
                 'cover': cover_path_str),
                 }
        groups = {'aid1': group1, 'aid2': group2, ...}
        
        tips: if group['entry'] isn't null in group1, aidl is actually sid.
        '''
        groups = {}
        for ret in videoSearch(self.path, self.pattern):
            if isinstance(ret, tuple):
                p = self.pattern[ret[0]]['parse']
                with open(ret[1], 'r', encoding='utf-8') as data:
                    aid = getByPath(json.load(data), p['aid'])
                try:
                    groups[aid]['body'].append(ret[1:])
                except KeyError:
                    groups[aid] = {'pattern': ret[0], 'entry': None, 'body': [ret[1:]], 'cover': ''}
            else:
                p = self.pattern[ret['pattern']]['parse']
                with open(ret['entry'], 'r', encoding='utf-8') as data:
                    if p['sid'] != None:
                        aid = getByPath(json.load(data), p['sid'])
                    else:
                        aid = getByPath(json.load(data), p['aid'])
                groups[aid] = ret
        return groups

    def __iter__(self):
        return self

    def __next__(self):
        if self.index < self.num:
            try:
                file = self.__core__(self.group[0].pop(), self.group[1], self.group[2])
            except (AttributeError, IndexError):
                group = self.groups[self.group_names.pop()]
                self.group[0] = group['body'].copy()
                self.count = len(self.group[0])
                self.group[1] = self.pattern[group['pattern']]['parse']
                if group['entry'] != None:
                    with open(group['entry'], 'r', encoding='utf-8') as data:
                        entry = json.load(data)
                    self.group[2] = (getByPath(entry, self.group[1]['title']),
                                     getByPath(entry, self.group[1]['aid']),
                                     getByPath(entry, self.group[1]['cover_url']),
                                     group['cover'])
                else:
                    self.group[2] = None
                return

            self.index += 1
            if os.path.exists(file):
                self.log.append("".join(["*" * 32, f"已完成{self.index - self.failed}个文件", "*" * 32, "\n"]))
                self.log[0] += 1
                return 0
            else:
                self.log.append("".join(["*" * 26, "合并失败: 文件未成功生成", "*" * 26, "\n"]))
                self.log[0] += 1
                self.failed += 1
                return 1
        else:
            raise StopIteration

    def __core__(self, group_body_item, pattern_parse, group_info=None):
        with open(group_body_item[0], 'r', encoding='utf-8') as data:
            entry = json.load(data)
        if group_info != None:
            title = group_info[0].translate(self.tabel).replace(".", "·")
            if group_info[1]:
                aid = group_info[1]
            else:
                aid = getByPath(entry, pattern_parse['aid'])
            cover_url = group_info[2]
            cover = group_info[3]
        else:
            title = getByPath(entry, pattern_parse['title']).translate(self.tabel).replace(".", "·")
            aid = getByPath(entry, pattern_parse['aid'])
            cover_url = getByPath(entry, pattern_parse['cover_url'])
            cover = ''
        cid = getByPath(entry, pattern_parse['cid'])
        part_title = getByPath(entry, pattern_parse['part_title']).translate(self.tabel).replace(".", "·")
        part_id = getByPath(entry, pattern_parse['part_id'])
        part_num = getByPath(entry, pattern_parse['part_num'])
        if self.switch2 and part_id != '':
            part_title = f"{part_id}_{part_title}"
        if self.switch5:
            _path = self.save_path
            cover_path = os.path.join(_path, 'coverJpg', f'{title}.jpg')
            if self.count > 1:
                part_title = title + '_' + part_title
            else:
                part_title = title
            if not self.switch6:
                same_count = 1
                title_bak = part_title
                while os.path.exists(os.path.join(_path, f'{part_title}.mp4')):
                    same_count += 1
                    part_title = title_bak + '(%s)' % same_count
                if same_count > 1:
                    cover_path = os.path.join(_path, 'coverJpg', f'{title}{same_count}.jpg')
            os.makedirs(os.path.join(_path, 'coverJpg'), exist_ok=True)
        else:
            _path = os.path.join(self.save_path, title)
            cover_path = os.path.join(_path, 'cover.jpg')
            if not self.switch6:
                same_count = 1
                title_bak = part_title
                while os.path.exists(os.path.join(_path, f'{part_title}.mp4')):
                    same_count += 1
                    part_title = title_bak + '(%s)' % same_count
                if same_count > 1:
                    cover_path = os.path.join(_path, f'cover{same_count}.jpg')
            os.makedirs(_path, exist_ok=True)
            self.log.append(f">>>输出目录：{title}")
            self.log[0] += 1
        # 分类创建ffmpeg进程进行合并
        output_file = os.path.join(_path, f'{part_title}.mp4')
        if group_body_item[2] == 'blv':
            self.__blv__(group_body_item[1], output_file)
            self.log.append(f">>>输出文件：{part_title}.mp4......")
        elif not os.path.exists(group_body_item[1]):
            self.log.append(f"|- - -缺少视频文件：{group_body_item[1]}")
        elif not os.path.exists(group_body_item[2]):
            self.log.append(f"|- - -缺少音频文件：{group_body_item[2]}")
        else:
            cmd = f'{self.source_path} -i "{group_body_item[1]}" -i "{group_body_item[2]}" -c copy -y "{output_file}"'
            # python调用Shell脚本执行cmd命令
            p = subprocess.Popen(cmd, shell=True, stdin=subprocess.DEVNULL, stdout= \
                subprocess.PIPE, stderr=subprocess.STDOUT)
            p.communicate()
            self.log.append(f">>>输出文件：{part_title}.mp4......")
        self.log[0] += 1
        # 调用danmaku2ass_light内建函数将xml弹幕文件转换成ass格式
        if self.switch and os.path.exists(group_body_item[3]):
            if self.danmu_config == None:
                dm = [23, 0.6, 0.667, 12.0, 6.0, 0]
            else:
                dm = self.danmu_config
            danmu_dir = os.path.join(_path, "danmuAss")
            os.makedirs(danmu_dir, exist_ok=True)
            danmu_file = os.path.join(danmu_dir, f"{part_title}.ass")
            Danmaku2ASS(group_body_item[3], "autodetect", danmu_file, \
                        1080, 720, reserve_blank=int(720 * dm[2]), font_size=int(dm[0]), text_opacity=dm[1],
                        duration_marquee=dm[3], duration_still=dm[4], is_reduce_comments=dm[5])
        # 调用CCdownload内建函数根据av和cv号请求字幕脚本并转换为srt格式
        if self.switch3 and aid != '' and cid != '':
            codetable = ["完成", "无字幕", "网络异常", "未检索到字幕"]
            srt_dir = os.path.join(_path, "ccSrt")
            os.makedirs(srt_dir, exist_ok=True)
            srt_file = os.path.join(srt_dir, f"{part_title}.srt")
            code = CCdownload.download(aid, cid, srt_file)
            self.log.append(f">>>字幕下载：{codetable[code]}")
            self.log[0] += 1
        # 保留缓存文件中的封面或者联网下载封面图片
        if self.switch4 and not os.path.exists(cover_path):
            if os.path.exists(cover):
                copy_file(cover_path, cover, buffer=256)
            else:
                ret = download(cover_url, cover_path)
                if not ret[0]:
                    self.log.append(f"|- - -封面下载失败：{ret[1]}")
                    self.log[0] += 1
        return output_file

    def __blv__(self, dirpath, filepath):
        filelist = os.listdir(dirpath)
        blvlist = [''] * len(filelist)
        txt_path = os.path.join(dirpath, 'concat.txt')
        for i in filelist:
            cuts = i.split('.')
            if cuts[-1] == 'blv':
                blvlist[int(cuts[0])] = i
            else:
                blvlist.pop()
        # 单个blv文件直接复制(更换容器)
        if len(blvlist) == 1:
            onefile = os.path.join(dirpath, blvlist[0])
            cmd = f'{self.source_path} -i "{onefile}" -c copy -y "{filepath}"'
            p = subprocess.Popen(cmd, shell=True, stdin=subprocess.DEVNULL, stdout= \
                subprocess.PIPE, stderr=subprocess.STDOUT)
            p.communicate()
        else:
            with open(txt_path, 'a', encoding='utf-8') as f:
                for i in blvlist:
                    file_path = os.path.join(dirpath, i)
                    f.write(f"file '{file_path}'\n")
            cmd = f'{self.source_path} -f concat -safe 0 -i "{txt_path}" -c copy -y "{filepath}"'
            p = subprocess.Popen(cmd, shell=True, stdin=subprocess.DEVNULL, stdout= \
                subprocess.PIPE, stderr=subprocess.STDOUT)
            p.communicate()
            os.remove(txt_path)
        return

    def __call__(self):
        print("<m4sMerge core>work at %s" % self.path)
        return {'Func': self.__class__.__name__, 'Core': self.source_path, 'Path': self.path, \
                'Danma': self.switch, 'Subtitle': self.switch3, 'Cover': self.switch4, \
                'Total': self.num, 'Done': self.index, 'Failed': self.failed}


class cmnfunc():
    '''
    同mainfunc类，用于合并任意音视频文件
    '''

    def __init__(self, FilePath, CorePath, ordered=False):
        if isinstance(FilePath, tuple):
            self.path = FilePath[0]
            self.save_path = os.path.join(FilePath[0], 'video_merge')
        else:
            self.path = FilePath
            self.save_path = os.path.join(os.path.dirname(FilePath), 'video_merge')
        self.source_path = CorePath
        self.switch = ordered
        self.log = [0, ]
        self.mes = [0, ]
        self.video_search()

    def video_search(self):
        include_v = ['flv', 'mkv', 'rmvb', '3gp', 'wmv']
        include_a = ['mp3', 'm4a', 'aac', 'wma', 'wav', 'ape', 'flac', 'cda']
        include = ['mp4', 'm4s', 'avi'] + include_v + include_a

        all_file = []
        for (a, b, c) in os.walk(self.path):
            for i in c:
                profix = i.split('.')[-1]
                if profix in include:
                    file_path = os.path.join(a, i)
                    all_file.append(file_path)

        self.all_video = [];
        self.all_audio = []
        left = all_file[:]
        for i in all_file[:-1]:
            left.remove(i)
            for j in left:
                cuts1 = os.path.basename(i).split('.')
                cuts2 = os.path.basename(j).split('.')
                if cuts1[:-1] == cuts2[:-1]:
                    if cuts2[-1] in include_a:
                        self.all_video.append(i)
                        self.all_audio.append(j)
                    elif os.path.getsize(i) >= os.path.getsize(j):
                        self.all_video.append(i)
                        self.all_audio.append(j)
                    else:
                        self.all_video.append(j)
                        self.all_audio.append(i)

        self.num = len(self.all_video)
        self.index = 0
        self.failed = 0
        if self.num == 0:
            self.group_num = 0
            self.mes.append("未找到可合并文件，请检查路径或文件格式。")
            self.mes[0] += 1
            return
        elif len(self.all_video) != len(self.all_audio):
            self.mes.append("检测到待合并文件存在缺失，可能会生成异常的视频文件。\n")
            self.mes[0] += 1
        self.group_num = 1
        os.makedirs(self.save_path, exist_ok=True)
        self.log.append("共检测到 %s 个待合并文件\n合并进程已开始，请耐心等待~~~\n" % self.num)
        self.log[0] += 1

    def __iter__(self):
        return self

    def __next__(self):
        if self.index<self.num:
            try:
                file = self.__core__()
                if os.path.exists(file):
                    self.log.append("".join(["*"*32,f"已完成{self.index+1-self.failed}个文件","*"*32,"\n"]))
                    self.log[0] += 1
                    return 0
                else:
                    self.log.append("".join(["*"*26,"合并失败: 文件未成功生成","*"*26,"\n"]))
                    self.log[0] += 1
                    self.failed += 1
                    return 1
            except Exception as e:
                self.log.append("".join(["*"*27,"合并失败: 遇到未知错误","*"*27]))
                self.log.append("Details: %s\n" % repr(e))
                self.log[0] += 2
                self.failed += 1
                return 2
            finally:
                self.index += 1
        else:
            raise StopIteration

    def __core__(self):
        include = ['flac', 'ape', 'wav', 'cda']
        cuts = os.path.basename(self.all_audio[self.index]).split('.')
        if self.switch:
            title = '%s_%s' % (self.index + 1, '.'.join(cuts[:-1]))
        else:
            title = '%s' % '.'.join(cuts[:-1])
        self.log.append(f">>>输出文件：{title}.mp4......")
        self.log[0] += 1
        output_file = os.path.join(self.save_path, f"{title}.mp4")
        if cuts[-1] in include:
            cmd = f'{self.source_path} -i "{self.all_video[self.index]}" -i "{self.all_audio[self.index]}" -c:v copy -c:a aac -strict experimental -map 0:v:0 -map 1:a:0 -y "{output_file}"'
        else:
            cmd = f'{self.source_path} -i "{self.all_video[self.index]}" -i "{self.all_audio[self.index]}" -c copy -map 0:v:0 -map 1:a:0 -y "{output_file}"'
        p = subprocess.Popen(cmd, shell=True, stdin=subprocess.DEVNULL, stdout= \
            subprocess.PIPE, stderr=subprocess.STDOUT)
        p.communicate()
        return output_file

    def __call__(self):
        print("<m4sMerge core>work at %s" % self.path)
        return {'Func': self.__class__.__name__, 'Core': self.source_path, 'Path': self.path, \
                'Total': self.num, 'Done': self.index, 'Failed': self.failed}


class ffmpegThread(Thread):
    '''
    绑定到一个MainWindow类实例的线程管理类
    创建一个线程，实现子进程的创建以及和父进程的交互
    '''

    def __init__(self, win, args):
        Thread.__init__(self, name='ffmpegThread')
        self.win = win
        self.paths = args
        self.win.source.wlog(self, 'Creat ffmpeg threading', 0)

    def info_pipe(self, task):
        if task.log[0] > 0:
            self.win.logger.AppendText('\n'.join(task.log[-task.log[0]:]) + '\n')
            task.log[0] = 0
        if task.mes[0] > 0:
            self.win.Mes('', task.mes[-1])
            task.mes[0] = 0

    def run(self):
        self.win.gauge.SetValue(4)
        try:
            if self.win.mode.GetValue() == 'bilibili':
                config = self.win.source.config
                task = mainfunc(*self.paths, self.win.source.pattern,
                                config.getint('config', 'danmu2ass'), config.getint('config', 'coverSave'),
                                config.getint('config', 'ccDown'), config.getint('config', 'videoNumber'),
                                config.getint('config', 'oneDir'), config.getint('config', 'overWrite'),
                                self.win.source.danmu_config)
            elif self.win.mode.GetValue() == '通用':
                task = cmnfunc(*self.paths)
            else:
                return
        except Exception:
            self.win.source.wlog(self, '<m4sMerge core>Initailization failed, mode:%s\n'
                                 % self.win.mode.GetValue() + format_exc(), 1)
            return
        else:
            self.win.source.wlog(self, f'<m4sMerge core>%s' % task(), 0)
            self.win.source.numRecord(task.group_num, 1)
        self.info_pipe(task)
        self.win.gauge.SetValue(8)
        value = ''
        for result in task:
            try:
                if result == None: continue
                while not self.win.state:
                    sleep(0.3)
                value += str(result)
                percent = 4 + int(94.0 * task.index / task.num)
                self.win.gauge.SetValue(percent)
                self.info_pipe(task)
            except Exception:
                self.info_pipe(task)
                task.index += 1
                task.failed += 1
                try:
                    errorFile = task.group[2][:2]
                    errorInfo = ' "%s", %s\n%s' % (*errorFile, format_exc())
                except TypeError:
                    errorFile = ('Unkown', 'Unkown')
                    errorInfo = '\n%s' % format_exc()
                self.win.source.wlog(self, '<m4sMerge core>Unknown error occurred when resolving or merging' + errorInfo, 1)
                if self.win.errorskip.IsChecked():
                    # 跳过未知错误，继续合并后面的文件
                    self.win.logger.AppendText("*" * 27 + "合并失败: 遇到未知错误" + "*" * 27 +
                                               f"\nFile Info:  (title: %s, aid: %s)\n\n" % errorFile)
                    continue
                self.win.logger.AppendText('合并线程已终止！\n(详细错误信息请查看日志文件)\n\n')
                return
        self.win.source.wlog(self, '<m4sMerge core>Return %s' % value, 0)
        self.win.source.wlog(self, '<m4sMerge core>%s' % task(), 0)
        self.win.source.numRecord(task.index - task.failed, 0)
        self.info_pipe(task)
        self.win.gauge.SetValue(100)
        # 完成提示
        if value != '' and int(value) == 0:
            self.win.logger.AppendText('--程序：所有文件都已经生成了！是一次成功的合并呢o(*￣▽￣*)ブ\n\n')
        else:
            self.win.logger.AppendText('当前任务已完成！\n\n')