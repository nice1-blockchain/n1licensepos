#!/bin/bash

# Define the Jungle4 API endpoint
JUNGLE4_API="https://jungle4.cryptolions.io:443"

# Define the test account and contract account names
TEST_ACCOUNT="n1licensepos"  # Replace with your test account name
CONTRACT_ACCOUNT="n1licensepos"  # Replace with your contract account name
PRODUCT_NAME="$(tr -dc '.12345abcdefghijklmnopqrstuvwxyz' < /dev/urandom | head -c 12)"
priceVar=$(( $RANDOM % 10 + 1 ))
PRICE="${priceVar} USDT"
TOKEN_CONTRACT="niceoneusdtt"
NFT_CONTRACT="simpleassets"
INT_REF=$RANDOM
EXT_REF=$RANDOM

# Test the 'setproduct' action
PRODUCT_DATA='["'$PRODUCT_NAME'","'$PRICE'","'$TOKEN_CONTRACT'","'$NFT_CONTRACT'",'$INT_REF','$EXT_REF']'
cleos -u $JUNGLE4_API push action $CONTRACT_ACCOUNT setproduct "$PRODUCT_DATA" -p $TEST_ACCOUNT@active

# Test the 'addproddata' action
sleep 5
NFT_OWNER="niceonedemos"
INT_PROD_REF=$INT_REF
ID=100000000012632
NFT_DATA='["'$NFT_OWNER'",'$INT_PROD_REF','$ID']'
cleos -u $JUNGLE4_API push action $CONTRACT_ACCOUNT addproddata  $NFT_DATA -p $TEST_ACCOUNT@active

# Test the 'delproduct' action
sleep 5
LABEL_OPTION="confirm"
DELPRODUCT_DATA='["'$PRODUCT_NAME'","'$LABEL_OPTION'"]'
cleos -u $JUNGLE4_API push action $CONTRACT_ACCOUNT delproduct $DELPRODUCT_DATA -p $TEST_ACCOUNT@active

# Test the 'deldata' action
sleep 5
DELDATA_DATA='["'$PRODUCT_NAME'","'$LABEL_OPTION'"]'
cleos -u $JUNGLE4_API push action $CONTRACT_ACCOUNT deldata $DELDATA_DATA -p $TEST_ACCOUNT@active

# Test the 'del1stock' action
sleep 5
ID_STOCK=100000000012626
STOCK_DATA='['$ID_STOCK']'
cleos -u $JUNGLE4_API push action $CONTRACT_ACCOUNT del1stock $STOCK_DATA -p $TEST_ACCOUNT@active

# Test the 'delsells' action
sleep 5
DELSELLS_JSON='["'$LABEL_OPTION'"]'
#cleos -u $JUNGLE4_API push action $CONTRACT_ACCOUNT delsells $DELSELLS_JSON -p $TEST_ACCOUNT@active

# Optionally, you can check contract table entries or transaction results to validate the test results
# Check the 'products' table
sleep 5
PRODUCT_LIST=$(cleos -u $JUNGLE4_API get table $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT products --limit 100 )
if [[ $PRODUCT_LIST =~ "\"product\": \"$PRODUCT_NAME\"" ]]; then
    echo "Product $PRODUCT_NAME exists: setproduct"
else
    echo "Product $PRODUCT_NAME does not exist: setproduct"
fi

# Check the 'productdata' table
sleep 2
PRODUCT_DATA=$(cleos -u $JUNGLE4_API get table $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT productdata --limit 100 )
if [[ $PRODUCT_DATA =~ "\"product\": \"$PRODUCT_NAME\"" ]]; then
    echo "Product $PRODUCT_NAME exists: addproddata works!"
else
    echo "Product $PRODUCT_NAME does not exist: addproddata has failed!"
fi

# Check the 'products' table to check del product. 
sleep 2
PRODUCT_LIST=$(cleos -u $JUNGLE4_API get table $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT products --limit 100 )
if [[ $PRODUCT_LIST =~ "\"product\": \"$PRODUCT_NAME\"" ]]; then
    echo "Product $PRODUCT_NAME exists: delproduct has failed!"
else
    echo "Product $PRODUCT_NAME does not exist: delproduct works!"
fi

# Check the 'productdata' table to check del data
sleep 2
PRODUCT_DATA=$(cleos -u $JUNGLE4_API get table $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT productdata --limit 100 )
if [[ $PRODUCT_DATA =~ "\"product\": \"$PRODUCT_NAME\"" ]]; then
    echo "Product $PRODUCT_NAME exists: deldata has failed!"
else
    echo "Product $PRODUCT_NAME does not exist: deldata works!"
fi

# Check the 'stock' table to check del stock
sleep 2
PRODUCT_DATA=$(cleos -u $JUNGLE4_API get table $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT stock --limit 100 )
if [[ $PRODUCT_DATA =~ "\"ID\": \"$ID_STOCK\"" ]]; then
    echo "ID $ID_STOCK exists: delstock has failed!"
else
    echo "ID $ID_STOCK does not exist: delstock works!"
fi

# Check the 'sells' table
#sleep 2
#PRODUCT_DATA=$(cleos -u $JUNGLE4_API get table $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT sells --limit 100)
#if [[ -z "$PRODUCT_DATA" ]]; then
#    echo "PRODUCT_DATA is empty or contains no elements: delsell works!"
#else
#    echo "PRODUCT_DATA is not empty: delsell has failed!"
#fi


echo "Unit tests completed."