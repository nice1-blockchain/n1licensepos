#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/symbol.hpp>
#include <eosio/string.hpp>
#include <eosio/system.hpp>

using namespace eosio;

class [[eosio::contract("n1licensepos")]] n1licensepos : public contract {
public:
  using contract::contract;


  //Set up a new product
    //product - Product name
    //price - Price of the product
    //tokencontract - Contract that manages the Tokens used for the purchase
    //nftcontract - Contract that manages the NFT 
    //int_ref - Establishes internal reference
    //ext-ref - Establishes external reference

  [[eosio::action]]
  void setproduct(name product, asset price, name tokencontract, name nftcontract, uint64_t int_ref, uint64_t ext_ref);


  //Capture the basic information of the product (NFT) to be sold.
    //owner - Current owner of the nft to capture data
    //int_ref - Internal product reference
    //id - Contract that manages the Tokens used for the purchase

  [[eosio::action]]
  void addproddata(name owner, uint64_t int_ref, uint64_t id);


  [[eosio::on_notify("simpleassets::transfer")]]
  void nft_transferin(name from, name to, std::vector<uint64_t>& assetids, std::string memo);


  [[eosio::on_notify("*::transfer")]]
  void incomingpay(name from, name to, asset quantity, std::string memo);

  [[eosio::action]]
  void deldata(name product, std::string memo) ;

    //Removes a product from the products table   
      //product - Product to be eliminated.
      //memo - "Confirm" for security

  [[eosio::action]]
  void delproduct(name product, std::string memo);


    //Removes a product from the stock table   
      //id - ID of the product to be eliminated.

  [[eosio::action]]
  void del1stock(uint64_t id);


    //Removes all data from the "sells" table.  
     //memo - "Confirm" for security

  [[eosio::action]]
  void delsells(std::string memo);

private:

  struct [[eosio::table]] product {
    name product;
    asset price;
    name tokencontract;
    name nftcontract;
    uint64_t int_ref;
    uint64_t ext_ref;

    uint64_t primary_key() const { return product.value; } 
    uint64_t by_int_ref() const { return int_ref; }
    uint64_t by_ext_ref() const { return ext_ref; } 
  };


  struct [[eosio::table]] productdata {
    name product;
    uint64_t basedon;
    name author;
    name category;
    string idata;

    uint64_t primary_key() const { return product.value; }
    
  };

  struct [[eosio::table]] stock {
    uint64_t ID;
    name producttype;
    bool sold;

    uint64_t primary_key() const { return ID; }
    uint64_t by_producttype() const { return producttype.value; } 
  };



  struct [[eosio::table]] sells {
    uint64_t id;       
    name product;      
    name sellto;       
    uint64_t saledate; 
    asset price;       

    uint64_t primary_key() const { return id; }
  };


typedef multi_index<"products"_n, product,
    indexed_by<"byintref"_n, const_mem_fun<product, uint64_t, &product::by_int_ref>>,
    indexed_by<"byextref"_n, const_mem_fun<product, uint64_t, &product::by_ext_ref>>
  > products_table;

typedef multi_index<"productdata"_n, productdata> productdata_table;

typedef multi_index<"stock"_n, stock,
    indexed_by<"byproducttype"_n, const_mem_fun<stock, uint64_t, &stock::by_producttype>> 
> stock_table;

typedef eosio::multi_index<"sells"_n, sells> sells_table;


  struct [[eosio::table]] sassets {
    uint64_t id;
    name owner;
    name author;
    name category;
    string idata;
   
    uint64_t primary_key() const { return id; }
  };

  typedef multi_index<"sassets"_n, sassets> sassets_table;
  
  
};

EOSIO_DISPATCH(n1licensepos, (setproduct))