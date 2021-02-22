a=0
while ((a<=1))
do 
  echo "input number a it must greater than 1" 
  read a
done
sum=0;
for j in $(seq 1 $a)
do 
  let sum+=j
done 
echo "1+,,+a==$sum"
