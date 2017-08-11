# 
# ~/.bashrc
#

if [[ -z "$DISPLAY" ]] && [[ $(tty) = /dev/tty1 ]]; then
    exec startx
fi

# If not running interactively, don't do anything
[[ $- != *i* ]] && return

alias ls='ls --color=auto'
GREEN="$(tput setaf 5)"
PS1='    ╔[\u@\h$PWD]\n    ╚>\$'
export TERMINAL=termite
screenfetch
