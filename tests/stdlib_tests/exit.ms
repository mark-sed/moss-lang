try {
    exit(42)
} catch (e:SystemExit) {
    "caught " ++ e.code ++ "\n"
}

"hi\n"
exit(0)
"not this\n"