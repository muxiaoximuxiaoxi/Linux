i=1
sum_while=0
while ((i<=100))
do 
  let sum_while+=i
  let ++i
done
echo "while:1+2+3,,,+100==$sum_while" 

j=1
sum_until=0
until ((j>100))
do 
  let sum_until+=j;
  let ++j
done 
echo "until:1+2+3,,,+100==$sum_until"


sum_for=0
for m in $(seq 1 100)
do 
  let sum_for+=m
done 
echo "for:1+2+3,,,+100==$sum_for"

