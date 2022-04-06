for i in {1..10}
do
    ./cli4 localhost 1234 <<< 'INSERT INTO Cars VALUES('$(($i + 10))', "HOLA", '$(($i + 5000))');' &
    
done
