# [配置文件说明](./) 

需要在当前目录中添加配置文件文件夹，例如：[conf](conf) 。也可以做成用户交互界面进行修改的文件，但是目前没有时间做这方面的更新，后序可以进行软件的迭代，完善这部分内容，目前只是通过配置文件完成的操作。 下面针对conf文件中的ini文件和json文件进行介绍。

## 1 ini 文件    

```json
{
    "?HostName": "当前主机名，用来标记读取当前活跃的json文件",
	"HostName": "Lenovo",
    "?RecentFile": "Json文件列表，也就是当前conf文件中的Json文件",
    "RecentFile": [  
        "Lenovo.json",
	    "BMW.json"
    ],
    "?Version": "版本号",    
    "Version": "v0.1.0",  
    "?Date": "更新日期",    
    "Date": "2023/05/28"
}
```

## 2 json文件   

```json
{
    "?ImagePath": "资源目录的路径，Desktop代表的是当前用户的桌面",
    "ImagePath": [
        {
            "Desktop": "Desktop"
        }
    ],
    "?TargetPath": "仓库目录列表，仓库名+路径",
    "TargetPath": [
        {
            "研发经理": "H:/00-sync-file-work/08-R&D-Manager-Learning"
        },
        {
            "泰国旅游": "F:/01-life-records/07-出国旅游"
        },
        {
            "Lenovo PngTest": "F:/05-markdown-assistant/test" 
        },
        {
            "购买记录": "F:/01-life-records/03-购买记录" 
        },
        {
            "生活相关": "F:/01-life-records" 
        }
    ],
    "?Interval": "间隔时间，目前是将当日的图片放到addList中，非当日的放到下面的delList中",
    "Interval": [
        "1 hours",
        "1 days",
        "2 days",
        "today"
    ],
    "?AssetsType": "支持的资源名，以及资源的类型",
    "AssetsType": [
        "99-*.png",
        "99-*.jpg",
        "99-*.bmp",
        "99-*.jpeg",
        "99-*.gif",
        "99-*.mp4"
    ],
    "?Software": "软件名+路径",
    "Software": {
        "Typora": "D:/Program Files/Typora/Typora.exe",
        "Vscode": "D:/Users/yang-wei-da/AppData/Local/Programs/Microsoft VS Code/Code.exe"
    }
}
```

## 3 更新配置   

默认情况下，一个电脑中，对应于一个ini文件和多个Json文件：

- ini文件中配置主机的名字，以及主机对应的json文件  
- json文件中需要将仓库的目录配置等信息配置  

>#### *tips：*
>
>1. 如果切换电脑的话，必须修改ini文件中的主机名字，以及编写新的json文件  
>1. json文件中，需要更新仓库的目录，以及软件的目录
