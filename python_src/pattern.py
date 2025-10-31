#!/usr/bin/env python3

'''
Module to construct Pattern semi-automatically.
The Pattern is an tree structure defined by python-dict, which tells main progrom
how to search and parse the files downloaded from different BiliBili clients.
User will be requested to give a specific path containing cache files and appiont
the meanings of key-values in entry file, during the process of Pattern builiding.
Other works like file recognization and tree generation will be completd by program
automatically.
It's a part of Bilibili Project by Yangwang.

Pattern demo:

tree = {'v':'video.mp4',
        'a':'audio1.mp4',
        'd':r'%group%_%episode%.xml',
        'e':r'%group%.info',
        'c':None,
        }
search = {'has_group':1, 'name': '.dvi', 'tree':tree}

parse = {'sid':'Season_id',
         'aid':'Aid',
         'bid':'Bid',
         'cid':'Cid',
         'title':'Title',
         'part_title':'PartName',
         'part_id':'PartNo',
         'part_num':'TotalParts',
         'cover_url':'CoverURL',
         }

pattern = {'name':'WIN10', 'search':search, 'parse':parse}
'''


import os
import re
import json
import subprocess


def dictLoop(root_dict, handleFunc, index=0, path=''):
    '''迭代字典中所有键值对并同时记录序号和路径'''
    for k in list(root_dict):
        if len(path) == 0:
            new_path = str(k)
        else:
            new_path = path + '-' + str(k)
        if not isinstance((root_dict[k]), dict):
            handleFunc(k, root_dict[k], index, new_path)
            index += 1
        else:
            index = dictLoop(root_dict[k], handleFunc, index, new_path)
    return index


def getByPath(dict, path):
    '''通过Pattern-parse中给出的字典路径访问字典'''
    if path == None: return ''
    value = dict
    try:
        for key in path.split('-'):
            value = value[key]
    except KeyError:
        value = ''
    else:
        if not (isinstance(value, str) or isinstance(value, int)):
            value = ''
    return value


class patternGenerator():

    def __init__(self, video_path, probe_path):
        self.path = video_path
        self.probe_path = probe_path
        self.file_path_tuple = self._fileRecognize()    # video, audio, entry, danmu, group_entry, cover

    def _fileRecognize(self):
        video = None
        audio = None
        entry = None
        danmu = None
        group_entry = None
        cover = None
        av_dir = None
        other_files = []
        prog = re.compile('handler_name *: [^: \n]+')
        for (a, b, c) in os.walk(self.path):
            other_files.clear()
            index = 0
            while True:
                file_type = 'Unkown'
                try:
                    file_path = os.path.join(a, c[index])
                except IndexError:
                    break
                # 首先通过ffprobe检测并识别出视频和音频文件
                p = subprocess.Popen(f'{self.probe_path} {file_path}', shell=True, stdin=subprocess.DEVNULL,
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                p_out, p_err = p.communicate()
                file_info = '\n'.join(str(p_out, encoding='utf-8').split('\r\n')[10:])
                # print(file_info)
                if re.match('Input', file_info):
                    try:
                        file_type = prog.search(file_info).group().split(': ')[1]
                    except AttributeError:
                        file_type = 'Image'
                # print(file_type)
                if file_type == 'VideoHandler':
                    av_dir = a
                    video = file_path
                elif file_type == 'SoundHandler':
                    audio = file_path
                else:
                    other_files.append(c[index])
                index += 1
            if video != None and audio != None: break
        if av_dir == None:
            # 未识别到视频文件则直接退出
            return (None, )
        # 检索标签和弹幕文件
        for f in other_files:
            file_path = os.path.join(av_dir, f)
            profix = f.split('.')[-1]
            if profix == 'xml':
                danmu = file_path
            else:
                try:
                    with open(file_path, 'r', encoding='utf-8') as fr:
                        data = json.load(fr)
                        if isinstance(data, dict):
                            entry = file_path
                except:
                    pass
        # 检索上级文件夹下的文件
        up_dir = os.path.dirname(av_dir)
        scandir_it = os.scandir(up_dir)
        with scandir_it:
            while True:
                try:
                    _entry = next(scandir_it)
                except StopIteration:
                    break
                if _entry.is_file():
                    profix = _entry.name.split('.')[-1]
                    if profix == 'jpg' or profix == 'jpeg':
                        cover = _entry.path
                    elif profix == 'xml':
                        danmu = _entry.path
                    else:
                        try:
                            with open(_entry.path, 'r', encoding='utf-8') as fr:
                                data = json.load(fr)
                                if isinstance(data, dict):
                                    # 若音视频文件所在目录下已有标签文件，则判断为组标签文件
                                    if entry == None:
                                        entry = _entry.path
                                    else:
                                        group_entry = _entry.path
                        except:
                            pass
        if group_entry != None and cover == None and \
                danmu != None and os.path.dirname(danmu) == up_dir:
            # 排除视频同目录下的json文件可能不是标签文件的情况
            entry = group_entry
            group_entry = None
        return video, audio, entry, danmu, group_entry, cover

    def _searchDefine(self):
        tip_list = ['\n未找到可识别的视频文件', '\n未找到可识别的音频文件', '\n未找到可识别的标签文件']
        for i in range(3):
            if self.file_path_tuple[i] == None:
                print(tip_list[i])
                return

        search = {'has_group': 0, 'name': None, 'tree': None}
        tree = {'v': None, 'a': None, 'd': None, 'e': None, 'c': None, }
        # 以视频标签文件所在位置为基准定位其他文件
        if self.file_path_tuple[4] != None:    # 含分组标签文件的情况
            search['has_group'] = 1
            dir_path = os.path.dirname(self.file_path_tuple[4])
            cut_pos = len(dir_path) + 1
            tree['v'] = '\\'.join(self.file_path_tuple[0][cut_pos:].split('\\')[1:])
            tree['a'] = '\\'.join(self.file_path_tuple[1][cut_pos:].split('\\')[1:])
            tree['e'] = '\\'.join(self.file_path_tuple[2][cut_pos:].split('\\')[1:])
            if self.file_path_tuple[3] != None:
                tree['d'] = '\\'.join(self.file_path_tuple[3][cut_pos:].split('\\')[1:])
            if self.file_path_tuple[5] != None:
                tree['c'] = self.file_path_tuple[5][cut_pos:]
            dir_name = os.path.basename(dir_path)
            entry_name_cut = os.path.basename(self.file_path_tuple[4]).split('.')
        else:
            dir_path = os.path.dirname(self.file_path_tuple[2])
            cut_pos = len(dir_path) + 1
            tree['v'] = self.file_path_tuple[0][cut_pos:]
            tree['a'] = self.file_path_tuple[1][cut_pos:]
            tree['e'] = self.file_path_tuple[2][cut_pos:]
            if self.file_path_tuple[3] != None:
                tree['d'] = self.file_path_tuple[3][cut_pos:]
            dir_name = os.path.basename(dir_path)
            entry_name_cut = os.path.basename(self.file_path_tuple[2]).split('.')

        # 格式化含上层文件夹名的文件名
        if dir_name == entry_name_cut[0]:
            search['name'] = '.%s' % entry_name_cut[-1]
        else:
            search['name'] = '.'.join(entry_name_cut)
        av_path = os.path.dirname(self.file_path_tuple[0])
        if dir_path != av_path:
            up_dir_name = dir_name
            dir_name = os.path.basename(av_path)
        else:
            up_dir_name = os.path.basename(os.path.dirname(dir_path))
        for k in ['e', 'd', 'c']:
            if tree[k] == None: continue
            pos = tree[k].find(up_dir_name)
            if pos >= 0:
                tree[k] = tree[k][:pos] + '%group%' + tree[k][pos+len(up_dir_name):]
            pos = tree[k].find(dir_name)
            if pos >= 0:
                tree[k] = tree[k][:pos] + '%episode%' + tree[k][pos+len(dir_name):]

        search['tree'] = tree
        return search

    def _keyDefine(self):

        def handleFunc(*items):
            if isinstance(items[1], str):
                value = items[1].replace('\n', ' ')
            else:
                value = items[1]
            display = f"{items[2] + 1}  {items[0]} = {value}"
            if len(display) > 56: display = display[:56]
            print(display)
            dict_parse_tmp.append(items[3])

        def keyAssign(parse_keys):
            if self.file_path_tuple[0] == None or \
                    self.file_path_tuple[1] == None or \
                    self.file_path_tuple[2] == None: return
            print("", "please enter the number of key-values above for each item:",
                  "(enter '0' when nothing matches, '-1' to go back.)", sep='\n')
            index = 0
            while True:
                try:
                    k = parse_keys[index]
                except IndexError:
                    break
                try:
                    n = int(input('  %s: ' % k))
                    if n == 0:
                        index += 1
                        continue
                    elif n < 0:
                        if index == 0:
                            raise IndexError
                        else:
                            index -= 1
                            continue
                    parse[k] = dict_parse_tmp[n-1]
                except (ValueError, IndexError):
                    print("Invaild number! please check or enter '0' to skip.")
                else:
                    index += 1
            print('\n')

        parse = {'aid': None, 'bid': None, 'cid': None, 'title': None,
                 'part_title': None, 'part_id': None, 'part_num': None, 'cover_url': None}
        dict_parse_tmp = []
        if self.file_path_tuple[4] != None:
            with open(self.file_path_tuple[4], 'r', encoding='utf-8') as data:
                info_dict = json.load(data)
            print('-'*54 + '\n' + "Group entry file '%s':" % self.file_path_tuple[4])
            dictLoop(info_dict, handleFunc)
            keyAssign(['sid', 'aid', 'bid', 'title', 'cover_url', 'part_num'])
        with open(self.file_path_tuple[2], 'r', encoding='utf-8') as data:
            info_dict = json.load(data)
        print('-'*54 + '\n' + "Video entry file '%s':" % self.file_path_tuple[2])
        dictLoop(info_dict, handleFunc, len(dict_parse_tmp))
        keyAssign([k for k in list(parse) if parse[k] == None])
        parse.setdefault('sid', None)
        return parse


def patternBuilding(path, probe_path):
    print('#' * 56, '#' + ' ' * 54 + '#', '#' + ' ' * 10 + 'Search-pattern Building Guide' + ' ' * 15 + '#',
          '#' + ' ' * 54 + '#', '#' * 56, sep='\n')
    print('Target path: %s' % path)
    print('\nsearching...\n')
    g = patternGenerator(path, probe_path)
    print('Found files:')
    try:
        for index, item in enumerate(['video', 'audio', 'entry', 'danmu', 'group entry', 'cover']):
            print(f'  {item}: {g.file_path_tuple[index]}')
    except IndexError:
        pass
    search = g._searchDefine()
    if search != None:
        parse = g._keyDefine()
        name = input('please enter the name of this pattern: ')
        pattern = {'name': name, 'search': search, 'parse': parse}
        print("\nPattern '%s' has created successfully." % name)
        return pattern
    print('\n' + '#' * 56)


if __name__ == '__main__':
    path = [r'E:\我的文档\新建\电脑端\94161637',
            r'E:\我的文档\新建\电脑端\UWP\s_959', r'E:\我的文档\新建\电脑端\UWP\94648758',
            r'E:\我的文档\新建\B站缓存\57209889']
    g = patternGenerator(path[0], r'E:\我的文档\新建\m4sMerge_v1.8\bin\ffmpeg\ffprobe.exe')
    files = g._fileRecognize()
    print('1. Found files:', files)
    search = g._searchDefine()
    print('2. Pattern-search:', search)
    parse = g._keyDefine()
    print('3. Pattern-parse:', parse)


