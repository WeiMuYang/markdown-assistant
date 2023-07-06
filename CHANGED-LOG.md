# [修改计划](./)  



## 待完成  

- 添加新建md按钮  
- 添加打开配置文件目录的功能   
- 添加文档编辑时的使用方法     
- 添加文件名修改或者路径修改，对于用的关联索引也改变  -->  并可视化显示   
- 添加自动整理apk文件的功能，添加文档归类模块     
- 打开配置文件按钮功能添加   

## 已完成  

- 配置文件的打开和修改  Json文件  
- 系统文件的打开和预览  ini文件  不能修改  
- 添加了刷新历史文件列表，将num的最大值改到 200 
- 修复单击列表中文件时，第二次但是找不到文件的bug    
- 添加最近修改的文件列表，但是单击有bug  
- 调整主界面，子目录的下拉菜单添加tooltip 
- 精简模式中添加 最近文件按钮   
- 配置文件中添加用户和电脑信息   
- 修改了读取文件的逻辑，读取文件的范围：当前路径下的所有子目录， 
- 解决了无法点击菜单栏的bug   
- 不再做功能更新，只做bug修复   持续使用3个月   
- 添加读取配置文件的功能  
- 重启功能   
- 同步和刷新合并成一个按钮   (重启功能  )   
- 添加about窗口  
- 添加批量资源文件导出功能，配置VScode软件的路径     
- 添加序号检索文件夹的功能，并可以用VScode打开相应序号的文件夹  
- 添加根据markdown代码，自动剪切/拷贝出资源文件  功能   
- 添加菜单栏，添加根据代码获取资源功能  
- 添加打开目录功能   
- 添加框选当前选中状态的功能      
- 添加状态栏，提示错误信息，和拷贝信息  
- 添加markdown代码 --> 添加居中功能 `<center>`
- 增加Typora的路径 --> 配置文件中   
- 界面需要调整    -> 低分辨率   
- 调整完分辨率，就不再更新  
- 添加使用Typora打开看最近文件的功能  https://blog.csdn.net/robertkun/article/details/21531967    
- 添加路径是否正确的状态图标  
- 路径错误，也能剪切成功-----bug修复  
- 使用仓库名索引整个仓库，自动刷新到最新的文档：目录/文件序号   
- Video添加起始帧的函数，返回mp4的第0帧  
- messageBox在子线程的发信号    
- 修改exe名字 ：Markdown Assistant   
- 界面调整  https://qtguide.ustclug.org/ch06-05.htm  
- MP4视频的触发方式需要添加：信号槽     
- 添加MP4加载到QLabel页面，添加ffmpeg模块        
- 弹出错误提示框，优化    
- 添加左右分割，添加Item的tooltip  
- 刷新后，Qlabel中的图片也需要刷新   -->  清空    
- JSON中次序存放path  
- 添加一个图标    
- 双击切换Item  
- 读取配置文件  
- 如果拖拽窗口，需要刷新图片大小   
- 添加自动读取目录下文档，将最后保存的文档序号放到 box中  
- 添加对mp4的支持  

## 不做的   

- 添加拖拽整理的功能，自动生成序号     不做 
- 添加时间：1小时，当天，前一天     
- 添加到托盘，或者添加精简窗口  -->  托盘隐藏      
- 给找不到的图片，添加默认图标     

## 乱码问题  

```C++
+//msvc编译器指定执行字符集
+msvc:QMAKE_CXXFLAGS += -execution-charset:utf-8

```

```C++

@@ -256,7 +256,7 @@ bool FileOperation::copyImgVideo(QDir& CurrentPath, const QStringList& fileNameA
     QStringList nameStrList;
     if(nameList.contains(",",Qt::CaseSensitive)){
         nameStrList = nameList.split(",");
-    }else if(nameList.contains("，",Qt::CaseSensitive)){
+    }else if(nameList.contains(QString::fromLocal8Bit("，"),Qt::CaseSensitive)){
         nameStrList = nameList.split("，");
     }else{
         nameStrList = nameList.split(" ");

```