#include <n1licensepos.hpp>



  //Set up a new product
    //product - Product name
    //price - Price of the product
    //tokencontract - Contract that manages the Tokens used for the purchase
    //nftcontract - Contract that manages the NFT 
    //int_ref - Establishes internal reference
    //ext-ref - Establishes external reference

  [[eosio::action]]
  void n1licensepos::setproduct(name product, asset price, name tokencontract, name nftcontract, uint64_t int_ref, uint64_t ext_ref) {
    require_auth(get_self()); 

    products_table products(get_self(), get_self().value);
    auto existing_product = products.find(product.value);
    check(existing_product == products.end(), "This product already exists");
    check(int_ref != ext_ref, "INT_REF and EXT_REF must be different");
    check(int_ref != 0, "INT_REF cannot be zero");

    auto int_ref_index = products.get_index<"byintref"_n>();
    auto existing_int_ref = int_ref_index.find(int_ref);
    check(existing_int_ref == int_ref_index.end(), "INT_REF already exists");
    check(ext_ref != 0, "EXT_REF cannot be zero");

    auto ext_ref_index = products.get_index<"byextref"_n>();
    auto existing_ext_ref = ext_ref_index.find(ext_ref);
    check(existing_ext_ref == ext_ref_index.end(), "EXT_REF already exists");

    products.emplace(get_self(), [&](auto &row) {
      row.product = product;
      row.price = price;
      row.tokencontract = tokencontract;
      row.nftcontract = nftcontract;
      row.int_ref = int_ref;
      row.ext_ref = ext_ref;
    });
  }


  //Capture the basic information of the product (NFT) to be sold.
    //owner - Current owner of the nft to capture data
    //int_ref - Internal product reference
    //id - Contract that manages the Tokens used for the purchase

  [[eosio::action]]
  void n1licensepos::addproddata(name owner, uint64_t int_ref, uint64_t id) {
    require_auth(get_self());

    sassets_table sassets("simpleassets"_n, owner.value);
    auto itr = sassets.find(id);

    check(itr != sassets.end(), "ID not found");

    name author = itr->author;
    name category = itr->category;
    string idata = itr->idata;

    products_table products(get_self(), get_self().value);
    auto product_itr = products.get_index<"byintref"_n>().find(int_ref);
    check(product_itr != products.get_index<"byintref"_n>().end(), "INT_REF not found");

    name product = product_itr->product;

    productdata_table productdata(get_self(), get_self().value);
    productdata.emplace(get_self(), [&](auto &row) {
     row.product = product;
     row.basedon = id;
     row.author = author;
     row.category = category;
     row.idata = idata;
    });
  }


  [[eosio::on_notify("simpleassets::transfer")]]
  void n1licensepos::nft_transferin(name from, name to, std::vector<uint64_t>& assetids, std::string memo) {
    if (to == get_self()) {
      uint64_t intRefFromMemo = std::stoull(memo);
      uint64_t ID_in = assetids[0];

      products_table products(get_self(), get_self().value);
      auto product_itr = products.get_index<"byintref"_n>().find(intRefFromMemo);
      check(product_itr != products.get_index<"byintref"_n>().end(), "INT_REF does not exist");

      name product = product_itr->product;

      productdata_table productdata(get_self(), get_self().value);
      auto productdata_itr = productdata.find(product.value);
      check(productdata_itr != productdata.end(), "Product not found in productdata");

      sassets_table sassets("simpleassets"_n, get_self().value);
      auto asset_itr = sassets.find(ID_in);

      if (asset_itr != sassets.end()) {
        check(asset_itr != sassets.end(), "No entry found with ID_in: " + std::to_string(ID_in) + ", nftfrom: " + from.to_string());

        check(asset_itr->author == productdata_itr->author, "The 'author' field does not match between sassets and productdata");
        check(asset_itr->category == productdata_itr->category, "The 'category' field does not match between sassets and productdata");
        check(asset_itr->idata == productdata_itr->idata, "The 'idata' field does not match between sassets and productdata");

        stock_table stock_tbl(get_self(), get_self().value);
        auto existing_stock = stock_tbl.find(ID_in);

        if (existing_stock == stock_tbl.end()) {
          stock_tbl.emplace(get_self(), [&](auto &row) {
            row.ID = ID_in;
            row.producttype = product;
            row.sold = false;
          });
        }
      }
    }
  }


  [[eosio::on_notify("*::transfer")]]
  void n1licensepos::incomingpay(name from, name to, asset quantity, std::string memo) {
    if (to == get_self()) {
      check(!memo.empty(), "Memo cannot be empty");

      products_table products(get_self(), get_self().value);
      auto product_itr = products.get_index<"byextref"_n>().find(std::stoull(memo));
      check(product_itr != products.get_index<"byextref"_n>().end(), "Does not match any product for sale");

      name token_contract = get_first_receiver();
      check(token_contract == product_itr->tokencontract, "Invalid issuing contract");

      asset price = product_itr->price;
      name tokencontract = product_itr->tokencontract;
      name nftcontract = product_itr->nftcontract;
      name product = product_itr->product;

      check(quantity == price, "Quantity received does not match product price");

      stock_table stock_tbl(get_self(), get_self().value);
      auto stock_itr = stock_tbl.get_index<"byproducttype"_n>().find(product.value);

      while (stock_itr != stock_tbl.get_index<"byproducttype"_n>().end() && stock_itr->sold == true) {
        ++stock_itr; // Mover al siguiente elemento
      }
      check(stock_itr != stock_tbl.get_index<"byproducttype"_n>().end(), "This product is out of stock.");

      uint64_t id_tosell = stock_itr->ID;
      std::vector<uint64_t> assetids_str;
      assetids_str.push_back(id_tosell);

      action(
        permission_level{get_self(), "active"_n},
        nftcontract,
        "transfer"_n,
        std::make_tuple(get_self(), from, assetids_str, std::string("Enjoy"))
      ).send();

      stock_tbl.get_index<"byproducttype"_n>().modify(stock_itr, get_self(), [&](auto &row) {
        row.sold = true;
      });

      sells_table sells_tbl(get_self(), get_self().value);
      sells_tbl.emplace(get_self(), [&](auto &row) {
        row.id = id_tosell;
        row.product = product;
        row.sellto = from;
        row.saledate = current_time_point().sec_since_epoch();
        row.price = price;
      });
    }
  }



    //Removes a product from the products table   
      //product - Product to be eliminated.
      //memo - "Confirm" for security

  [[eosio::action]]
  void n1licensepos::delproduct(name product, std::string memo) {
    require_auth(get_self()); 

    check(memo == "confirm", "FAIL");

    products_table products(get_self(), get_self().value);
    auto product_itr = products.find(product.value);
    check(product_itr != products.end(), "Product not found in 'products' table");

    products.erase(product_itr);
  }

    //Removes a product from the productdata table   
      //product - Product to be eliminated.
      //memo - "Confirm" for security

  [[eosio::action]]
  void n1licensepos::deldata(name product, std::string memo) {
    require_auth(get_self());

    check(memo == "confirm", "FAIL");

    n1licensepos::productdata_table productdata(get_self(), get_self().value);
    auto data_itr = productdata.find(product.value);
    check(data_itr != productdata.end(), "Product not found in table 'productdata'");

    productdata.erase(data_itr);
}


    //Removes a product from the stock table   
      //id - ID of the product to be eliminated.

  [[eosio::action]]
  void n1licensepos::del1stock(uint64_t id) {
    require_auth(get_self()); 
    stock_table stock_tbl(get_self(), get_self().value);
    auto stock_itr = stock_tbl.find(id);

    check(stock_itr != stock_tbl.end(), "ID not found in table 'stock'.");

    stock_tbl.erase(stock_itr);
  }


    //Removes all data from the "sells" table.  
     //memo - "Confirm" for security

  [[eosio::action]]
  void n1licensepos::delsells(std::string memo) {
    require_auth(get_self());

    check(memo == "confirm", "FAIL: El memo debe ser 'confirm'");

    sells_table sells_tbl(get_self(), get_self().value);

    auto itr = sells_tbl.begin();
    while (itr != sells_tbl.end()) {
        itr = sells_tbl.erase(itr);
    }
  }