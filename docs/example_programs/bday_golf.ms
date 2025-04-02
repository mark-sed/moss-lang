fun m(i,s){r="";for(_:0..i)r++=s;return r;}
t=Int(input("> "))
d="----"
s="  "
a="+\n"
r=m(t-1,s)
r++"  * *\n"++r++" _|_|_\n"++r++"|     |\n"
for(i:2..t+1){
    b=m((t-i),s)
    b++"+-"++m(i,d)++a++b++"| "++m(i*2,s)++"|\n"
}
"+-"++m(t,d)++a++"\nHappy "++t++". Birthday, Moss!\n"