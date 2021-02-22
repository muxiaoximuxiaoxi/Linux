touch awk1.txt
touch awk2.txt 
awk -f '$0/abc/ {printf$1}'/ test.log > awk1.txt  
sort -n ./awk1.txt |uniq -c |sort -n >awk2.txt 
awk '$1>10 {printf$2}' awk2.txt 

