

## git 分支管理

查看分支：git branch

创建分支：git branch <name>

切换分支：git checkout <name>或者git switch <name>

创建+切换分支：git checkout -b <name>或者git switch -c <name>

合并某分支到当前分支：git merge <name>

删除分支：git branch -d <name>

用git log --graph命令可以看到分支合并图。

合并分支时，加上--no-ff参数就可以用普通模式合并，合并后的历史有分支，能看出来曾经做过合并，而fast forward合并就看不出来曾经做过合并。

当手头工作没有完成时，先把工作现场git stash一下，然后去修复bug，修复后，再git stash pop，回到工作现场；
用git stash list命令看看存储的工作现场
一是用git stash apply恢复，但是恢复后，stash内容并不删除，你需要用git stash drop来删除；
在master分支上修复的bug，想要合并到当前dev分支，可以用git cherry-pick <commit>命令，把bug提交的修改“复制”到当前分支，避免重复劳动。

Git专门提供了一个cherry-pick命令，让我们能复制一个特定的提交到当前分支：
提交的master中修复的bug的提交复制到dev分值

如果要丢弃一个没有被合并过的分支，可以通过git branch -D <name>强行删除。


## git remote
要查看远程库的信息，用git remote：
用git remote -v显示更详细的信息：


git push origin master

## 多人团队合作

因此，多人协作的工作模式通常是这样：

首先，可以试图用git push origin <branch-name>推送自己的修改；

如果推送失败，则因为远程分支比你的本地更新，需要先用git pull试图合并；

如果合并有冲突，则解决冲突，并在本地提交；

没有冲突或者解决掉冲突后，再用git push origin <branch-name>推送就能成功！

如果git pull提示no tracking information，则说明本地分支和远程分支的链接关系没有创建，用命令git branch --set-upstream-to <branch-name> origin/<branch-name>。

这就是多人协作的工作模式，一旦熟悉了，就非常简单。

小结
查看远程库信息，使用git remote -v；

本地新建的分支如果不推送到远程，对其他人就是不可见的；

从本地推送分支，使用git push origin branch-name，如果推送失败，先用git pull抓取远程的新提交；

在本地创建和远程分支对应的分支，使用git checkout -b branch-name origin/branch-name，本地和远程分支的名称最好一致；

建立本地分支和远程分支的关联，使用git branch --set-upstream branch-name origin/branch-name；

从远程抓取分支，使用git pull，如果有冲突，要先处理冲突。

rebase操作可以把本地未push的分叉提交历史整理成直线；

rebase的目的是使得我们在查看历史提交的变化时更容易，因为分叉的提交需要三方对比。


### git标签

命令git tag <tagname>用于新建一个标签，默认为HEAD，也可以指定一个commit id；

命令git tag -a <tagname> -m "blablabla..."可以指定标签信息；

命令git tag可以查看所有标签。
命令git push origin <tagname>可以推送一个本地标签；

命令git push origin --tags可以推送全部未推送过的本地标签；

命令git tag -d <tagname>可以删除一个本地标签；

命令git push origin :refs/tags/<tagname>可以删除一个远程标签。




  1 ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQC550LKTV9mMiHlzNsKjC7V1uh7PN5ZDv6UW6z/Yga245BOgazfctQbcBLelrmjqkAuFQraKxxVdLf6Yh9h0JbDVDtQGTZVnE6DR6l9uDt5E2/W5Yp    2OScAvf4SnJGlUydgA3nbGSIZ9AxUmhPk3sGLBWV8Da36xgw9May0q+2xv9U05yULqCAEe/0HAN6iNdFssTnLnRmSFiiWDXb+pQhGc9+rp0sFNtIS8LJCZGytwyX/6Vsd1zXIvbDUPDOsEEqVMRfUem    7O8eWsfA+SJA8UPDlLc1QtgE6U2/ifZ44g6QRIFPg4y+yWzJ/AhLV9VuloWMS7av8GAvjIOa7m4udT 2587233023@qq.com