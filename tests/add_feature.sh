#!/bin/bash


function check()
{
    if [ "$1" == "$2" ]
    then
        echo -e "\n\t\t\t\t\t\t\t\t[✔] Success"
    else
        echo -e "\n\t\t\t\t\t\t\t\t[X] Failed!"
        exit -1
    fi
}



function init_default()
{
    ../bin/pm create_project $1 &> /dev/null
    check $? $2
}


function check_dir()
{
    ls $1 > /dev/null 2> /dev/null
    check $? $2
}

# check if subdirectories exist
function check_sub_dirs()
{
    sum=0
    ls $1/src &>  /dev/null
    sum=$(($sum+$?))
    ls $1/bin &>  /dev/null
    sum=$(($sum+$?))
    ls $1/lib &>  /dev/null
    sum=$(($sum+$?))
    ls $1/conf &>  /dev/null
    sum=$(($sum+$?))
    ls $1/tests &>  /dev/null
    sum=$(($sum+$?))

    check $sum $2
}


function add_feature()
{
    cd feature_test
    ../../bin/pm add_feature "$1"  &> /dev/null
    check $? $2
    cd ..
}

rm -rf feature_test


echo -n "Creating project... "
init_default feature_test 0

echo -n "Adding feature... "
add_feature feature1 0

echo -n "Checking if feature is added twice... "
add_feature feature1 17


echo -n "Checking if project crashes on invalid characters... "
add_feature '][/#;  ]' 22


echo -n "Checking if feature dir was created... "
check_dir feature_test/feature1 0

echo -n "Checking if subdirectories were created..."
check_sub_dirs feature_test/feature1 0



