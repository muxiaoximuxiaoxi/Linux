echo "input number a"
read a
if(($a>10));then
  echo "%a>10"
elif(($a==10));then 
  echo "$a==10"
elif(($a==9));then
  echo "$a==9"
else
  echo "a<10&&a!=9"
fi
