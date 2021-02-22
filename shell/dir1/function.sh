function hello()
{
  echo "hello"
}
hello
function add()
{
  #sum=$($1+$2)
  let sum=$1+$2
  echo "$sum"
}
add $1 $2
