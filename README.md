# [Markdown助手](./)   [img](./img)  

## 1 简介 

Markdown助手的主要是用于Markdown文件编写过程中，资源的管理以及文件的批量修改，可以帮助Markdown的编辑人员，更加专注于文件的编写，提高文档的编辑效率。如果当前网页图片加载不下来，可以移步[CSDN博客介绍](https://blog.csdn.net/tangsenghandan/article/details/130914281?csdn_share_tail=%7B%22type%22%3A%22blog%22%2C%22rType%22%3A%22article%22%2C%22rId%22%3A%22130914281%22%2C%22source%22%3A%22tangsenghandan%22%7D)。

<center>    
<img src=./img/01-1.png alt=img-01-1 width=80%;/>   
</center>    

由于进行资料整理时，采用仓库的的方式进行文档的编辑和管理，因此，可以利用这个工具进行文档编辑和资料整理: 

- 资源: 获取Image，Video和Audio的位置  
- 仓库: 当前编辑文档的位置  

### 1.1 基本操作 

这个软件的功能可以将获取的资源文件，拷贝到想要的目录中，然后生成相应的Markdown代码，再将Markdown代码复制到文档中。操作举例:  

<center>    
<img src=./img/01-1.gif alt=img-01-1 width=80%;/>   
</center>    

### 1.2 软件环境   

- 开发平台: Windows 10 
- 开发环境: Qt Creator 11.0.1
- 开发语言: C++ 
- 外部库: Qt 5.12.0以及 [ffmpeg](https://ffmpeg.org//download.html#build-windows)   

## 2 功能介绍  

### 2.1 操作界面  

主界面中的包含：菜单栏、资源列表、文件操作和日志信息和历史文件这五大部分。下面针对五部分进行演示。

### 2.2 菜单栏   

#### 2.2.1 文件   

<center>    
<img src=./img/01-2.png alt=img-01-2 width=49%;/>   
</center>    

- 修改系统文件和添加配置文件：可以对用户的配置文件进行修改，包括：软件配置文件`*.ini` 和用户配置文件`*.json`        
- 配置文件: 配置文件可以通过`添加配置文件`手动的修改，也可以通过 `当前配置`来进行修改 

<center>    
<img src=./img/01-3.png alt=img-01-3 width=80%;/>   
</center>    

- 打开当前目录和资源目录: 打开相应的文件夹
- 配置文件列表: 可以切换当前的配置文件   

<center>    
<img src=./img/01-3.gif alt=img-01-3 width=80%;/>   
</center>    

- 重启：重新启动软件    

#### 2.2.2 编辑    

<center>    
<img src=./img/01-4.png alt=img-01-4 width=80%;/>   
</center>    

- 资源操作: 对资源文件列表进行插入和删除等操作  
- 仓库操作: 对仓库目录进行插入删除，以及对子目录和父目录进行添加操作  
- 重命名: 对文件或者目录进行重命名，同时可以将引用的位置也进行修改，重命名可以通过字符替换，通过列表的方式批量改名，以及自动加编号等方式  

<center>    
<img src=./img/01-4.gif alt=img-01-4 width=80%;/>   
</center>    

- 获取相对路径: 可以将历史文档中的文件相对路径，拷贝到当前的文件中  

<center>    
<img src=./img/01-5.gif alt=img-01-5 width=80%;/>   
</center>    

- 刷新日志和历史文件: 将日志信息清空，刷新当前仓库中的文件按照修改时间排序的前20个 
- 刷新子目录: 刷新当前子目录中的文件按照修改时间排序的前20个   
- 获取资源：通过Markdown代码自动索引相应的资源，并拷贝到资源目录的Picture文件夹下      

<center>    
<img src=./img/01-7.gif alt=img-01-7 width=80%;/>   
</center>  

#### 2.2.3 视图 

- 极简窗口：可以将软件界面缩小，仅仅展示添加的资源和当前文件信息，同时置于最前端  
- 最上面显示: 将在所有窗口的最前面显示窗口   

<center>    
<img src=./img/01-6.gif alt=img-01-6 width=80%;/>   
</center>    

#### 2.2.4 帮助   

<center>    
<img src=./img/01-8.gif alt=img-01-8 width=80%;/>   
</center>    

目前只是显示版本信息，如果有其他问题，可以邮件联系作者本人。欢迎进行技术交流。

### 2.3 资源列表    

<center>    
<img src=./img/01-9.gif alt=img-01-9 width=80%;/>   
</center>    

- 刷新：刷新资源列表。上面的列表是需要添加到Markdown中的资源列表，下面的列表是不放到Markdown中的资源。      
- 同步：单击资源，可以选中，上面的列表和下面的列表如果有选中的会互换位置     
- 图标：列表中的资源可以显示list方式，也可以使用大图标模式    
- 双击资源名：可以将资源调整到另一个列表中  
- 上面的list是真正要添加到Markdown中的资源  
- 右击菜单: 实现资源的操作，也可以修改导入资源文件代码的中占页面的比例 

### 2.4 文件操作   

<center>    
<img src=./img/01-10.gif alt=img-01-10 width=80%;/>   
</center>    

- 资源：资源的目录，一般是截图的图片或者视频等资源  
- 仓库：当前编辑文档库所在的仓库 ，仓库后面是当前仓库的子目录列表：01-study  等   
- 打开：打开当前的文件   
- 最近文件：最近修改的文件   
- 后面的数字：代表当前文件的编号   
- 创建文件: 创建新文件和新子目录   

<center>    
<img src=./img/01-11.gif alt=img-01-11 width=80%;/>   
</center>    

> #### ***注意：***   
>
> 仓库中的Markdown文件需要用01-名字，进行编号，这样既方便进行编号管理，有方便文档的管理。例如：
>
> <center>    
> <img src=./img/01-7.png alt=img-01-6 width=49%;/>    
> <img src=./img/01-6.png alt=img-01-7 width=45%;/>   
> </center>    

- 剪切：将资源文件剪切到当前文档所在的img/video/audio目录下，会自动筛查文件编号，添加新的文件  
- 剪切后可以在将Markdown的代码放到剪切板中，以png为例  ： 
```markdown
<center>    
<img src=./img/19-8.png alt=19-8 style=width:50%;/>   
</center>   
```

### 2.5 显示  

- 显示：是当前资源的缩略图，目前支持图片和视频的预览   
- 可以设置占在页面宽度的百分比 

<center>    
<img src=./img/01-12.gif alt=img-01-12 width=80%;/>   
</center>    

### 2.6 日志信息与历史文件 

- 日志: 当前操作的记录   

### 2.7 历史文件

- 历史文件: 显示当前仓库历史文件的top 20， Ctrl + L 显示当前子目录的 top 20  

<center>    
<img src=./img/01-13.gif alt=img-01-13 width=80%;/>   
</center>    

- 双击即可打开，Ctrl + C即可复制相对路径   

<center>    
<img src=./img/01-14.gif alt=img-01-14 width=80%;/>   
</center>    
### 2.8 托盘功能  

关闭窗口的话，需要从托盘进行退出。同时也可以有三种图标可以切换。

<center>    
<img src=./img/01-9.png alt=img-01-9 width=80%;/>   
</center>    


## 3 配置文件  

配置文件对应的目录是 [conf](conf) 。这部分功能后期可以通过可视化界面的方式方便用户进行自主的更新和配置，目前先通过配置文件进行修改相关内容。配置文件包含两部分：ini文件和json文件。对于文件的详细说明可以参考文档： [CONFIG.md](CONFIG.md) 。 

## 4 开发计划  

计划开发一套完善的Markdown助手软件，需要迭代的功能进行了整理，如果有哪些功能见开发文档：[CHANGED-LOG.md](CHANGED-LOG.md)   

## 5 关于作者  

该软件的开发人员是本人完成，由于平时工作忙碌，还有其他有待开发的其他功能，而且对于功能的测试还不完善，欢迎感兴趣的coder可以加入进来。也可以通过邮件的方式进行技术交流：863255386@qq.com。  
