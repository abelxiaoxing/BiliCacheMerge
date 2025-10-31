#!/usr/bin/env python3

# Module to download Closed Caption Subtitles from 'api.bilibili.com'.
# It's a part of program 'm4sMerge_bili' by Yangwang.

from requests import get
from time import gmtime, strftime

header = {
    "Host": 'api.bilibili.com',
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:67.0) Gecko/20100101 Firefox/67.0',
    'Accept': '*/*',
    'Accept-Language': 'zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2',
    'Accept-Encoding': 'gzip, deflate, br',
    'Connection': 'keep-alive'
    }

def download(aid, cid, path):
    url = 'https://api.bilibili.com/x/web-interface/view?aid=%s&cid=%s'\
          % (aid,cid)
    try:
        r = get(url, header, timeout=3)
    except Exception:
        return 2
    data = r.json()
    if data['code'] != 0:
        return 3
    jslist = data['data']['subtitle']['list']
    if len(jslist) == 0:
        return 1
    for item in jslist:
        lan = item['lan']
        if len(jslist) > 1:
            path = path.split('.')[0] + '_%s.srt' % lan
        _url = item['subtitle_url']
        subtitle_data = get(_url).json()
        ccList2srt(subtitle_data['body'], path)
    return 0

def ccList2srt(cclist, path, **font_args):
    def timeTrans(time):
        cut = str(time).partition('.')
        intTime = strftime('%H:%M:%S', gmtime(time))
        digits = len(cut[2])
        if digits == 0: decTime = '000'
        elif digits == 1: decTime = cut[2] + '00'
        elif digits == 2: decTime = cut[2] + '0'
        else: decTime = cut[2]
        return f'{intTime}.{decTime}'
    fn = font_args.get('fn')
    fs = font_args.get('fs', 14)
    fc = font_args.get('fc')
    if fn != None: head = r'{\fn%s\fs%s}' % (fn, fs)
    else: head = r'{\fs%s}' % fs
    if fn != None: head = '<font color=%s>' % fn + head
    with open(path, 'w', encoding='utf-8') as f:
        for i, line in enumerate(cclist):
            tag = f"{i+1}\n{timeTrans(line['from'])} --> \
{timeTrans(line['to'])}\n"
            _line = tag + head + line['content'] + '\n\n'
            f.write(_line)

if __name__ == '__main__':
    download(584339009, 225750427, r'E:\我的文档\新建\bilibili_new\test.srt')
