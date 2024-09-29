#!/usr/bin/env janet

(import fastcgi)

(fastcgi/accept)
(fastcgi/print-err "test-error\n")
(fastcgi/print-out
  (string
    "Content-type: text/html\r\n"
    "\r\n"
    "header: "
    (fastcgi/get-param "TEST_HEADER")
    "\n"
    "content: "
    (fastcgi/read-all)
    "\n"))
(fastcgi/accept)
