# Detecting Cryptocurrency Fraud with Graph Embeddings

This demo illustrates the use of TigerGraph and ML models to identify phishing accounts from cryptocurrency transactions. It demonstrates the following
1. How to load data into TigerGraph database
2. How to compute graph embeddings using the TigerGraph Data Science Library algorithms
3. How to train a neural network model with the learned embeddings

## Getting Started

The quickest way to experience with TigerGraph is to run its docker container. That is also the approach taken by this demo. However, this demo only requires access to an machine with TigerGraph running on it. 

1. Create a folder `~/embedding_demo` on your machine. This will be the working directory for this demo. You can create a folder with any name at any location, but remember to use that instead in the rest of the demo. If you use terminal, run `mkdir ~/embedding_demo`.
3. Inside `~/embedding_demo`, clone this repo. On terminal
```
cd ~/embedding_demo
git clone https://github.com/TigerGraph-DevLabs/detect-cryptocurrency-fraud.git
```
3. Go into the cloned folder and install the required python packages. It is recommended to create a virtual enviroment for python first but it is not necessary. (This demo uses python >=3.6, so older versions of python might not work.) On termial
```
cd ~/embedding_demo/detect-cryptocurrency-fraud
pip install -r requirements.txt
```
5. Follow the [offical guide](https://docs.tigergraph.com/start/get-started/docker) to start TigerGraph in docker container. Note: when running the container, map the local directory `~/embedding_demo` to `/home/tigergraph/embedding_demo` instead of using the default values in the guide. Make sure you finish all the steps in the guide and you can see the GraphStudio at `http://localhost:14240` in your browser.

## Data

The graph data comes from (XBlock)[http://xblock.pro/ethereum/#EPT]. It is collected from the Ethereum platform, home to the second largest cryptocurrency, Ether (ETH), by Chen *et al*. Vertices in the graph are Ethereum accounts (also called wallets) and edges are transactions between the accounts. Each vertex has an attribute `is_fraud` denoting whether or not the vertex is a phishing account. 

As the data files are too large to be included in this repo, they are to be downloaded at:
* (accounts.csv)[https://tigergraph-public-data.s3.us-west-1.amazonaws.com/Ethereum/accounts.csv] contains data about vertices (accounts)
* (transactions.csv)[https://tigergraph-public-data.s3.us-west-1.amazonaws.com/Ethereum/transactions.csv] contains data about edges (transactions)

If you are on the terminal, run the following to download
```
cd ~/embedding_demo/detect-cryptocurrency-fraud/data
wget https://tigergraph-public-data.s3.us-west-1.amazonaws.com/Ethereum/accounts.csv
wget https://tigergraph-public-data.s3.us-west-1.amazonaws.com/Ethereum/transactions.csv
```

Finally, we will load the data into TigerGraph. Make sure your TigerGraph is up and running. Then SSH into the TigerGraph server
`ssh -p 14022 tigergraph@localhost`. If there is any issue loggin in, please refer to the [offical guide](https://docs.tigergraph.com/start/get-started/docker). 

After logged in, go into the data folder and run the GSQL script `load_data.gsql` to load data into the database.
```
# Inside the TigerGraph docker
cd ~/embedding_demo/detect-cryptocurrency-fraud/data
gsql load_data.gsql
```
That script will create the vertex and edge schemas and load the files into the database. You can open the script with any text editor and check out the gsql commands. After the loading job finishes, you will see the graph in GraphStudio at `http://localhost:14240`. 




