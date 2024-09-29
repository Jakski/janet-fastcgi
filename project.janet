(declare-project
  :name "fastcgi"
  :description "Binding to libfcgi"
  :author "Jakub Pieńkowski"
  :license "UNLICENSE"
  :url "https://github.com/Jakski/janet-fastcgi"
  :repo "https://github.com/Jakski/janet-fastcgi.git")


(declare-native
  :name "fastcgi"
  :ldflags ["-lfcgi"]
  :cflags ["-Wall"]
  :source ["fastcgi.c"])
