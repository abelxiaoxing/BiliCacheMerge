#ifndef QTBILIMERGE_H
#define QTBILIMERGE_H

// 版本信息
#define QTBILIMERGE_VERSION "1.0.0"
#define QTBILIMERGE_VERSION_MAJOR 1
#define QTBILIMERGE_VERSION_MINOR 0
#define QTBILIMERGE_VERSION_PATCH 0

// 项目信息
#define QTBILIMERGE_NAME "Qt B站缓存合并工具"
#define QTBILIMERGE_DESCRIPTION "基于Qt的B站缓存文件合并工具"

// 配置文件路径
#define CONFIG_FILE_NAME "config.ini"
#define LOG_FILE_NAME "qt_bili_merge.log"
#define DEFAULT_OUTPUT_DIR "bili_merge"

// 支持的文件格式
#define SUPPORTED_VIDEO_EXTENSIONS "*.mp4 *.m4s *.avi *.flv *.mkv *.wmv *.3gp *.rmvb *.blv"
#define SUPPORTED_AUDIO_EXTENSIONS "*.m4a *.aac *.mp3 *.wav *.wma *.ape *.flac *.cda"
#define SUPPORTED_ALL_EXTENSIONS "*.mp4 *.m4s *.avi *.flv *.mkv *.wmv *.3gp *.rmvb *.blv *.m4a *.aac *.mp3 *.wav *.wma *.ape *.flac *.cda"

// 默认设置
#define DEFAULT_FONT_SIZE 23
#define DEFAULT_TEXT_OPACITY 0.6
#define DEFAULT_REVERSE_BLANK 0.667
#define DEFAULT_DURATION_MARQUEE 12.0
#define DEFAULT_DURATION_STILL 6.0
#define DEFAULT_IS_REDUCE_COMMENTS 0

// FFmpeg相关常量
#define FFMPEG_EXECUTABLE_NAME "ffmpeg"
#define FFPROBE_EXECUTABLE_NAME "ffprobe"

// 网络请求相关
#define REQUEST_TIMEOUT 30000
#define MAX_RETRIES 3

// UI相关常量
#define MAIN_WINDOW_MIN_WIDTH 500
#define MAIN_WINDOW_MIN_HEIGHT 600
#define MAIN_WINDOW_DEFAULT_WIDTH 600
#define MAIN_WINDOW_DEFAULT_HEIGHT 700
#define SETTINGS_DIALOG_WIDTH 560
#define SETTINGS_DIALOG_HEIGHT 640

// 进度条范围
#define PROGRESS_MIN 0
#define PROGRESS_MAX 100

// 日志级别
enum LogLevel {
    LOG_INFO = 0,
    LOG_WARNING = 1,
    LOG_ERROR = 2,
    LOG_DEBUG = 3
};

// 合并模式
enum MergeMode {
    BILIBILI_MODE = 0,
    UNIVERSAL_MODE = 1
};

// 合并结果
enum MergeResult {
    MERGE_SUCCESS = 0,
    MERGE_FAILED = 1,
    MERGE_ERROR = 2
};

#endif // QTBILIMERGE_H