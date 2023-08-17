#!/bin/sh

this_tool_dir_bn="99.sync_to_gitee_pth"
dest_path=/home/greenhand/20.working/20.x-optical/01.mDR/91.gitee_rep/dr-manger-group/软件/90.src/02.route_board/02.dr_manager
rsync -rpt . "$dest_path"/
rm -rf "$dest_path"/.cache "$dest_path"/.git "$dest_path"/compile_commands.json "$dest_path"/"$this_tool_dir_bn"

