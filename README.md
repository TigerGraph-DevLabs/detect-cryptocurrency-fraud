# Detecting Cryptocurrency Fraud with Graph Embeddings

This demo illustrates the use of TigerGraph and ML models to identify phishing accounts from cryptocurrency transactions. It demonstrates the whole end-to-end experience including:
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

## Graph Embedding

First, we will install the two graph embedding algorithms - node2vec and fastRP - to the graph we just created. If you have already installed the [TigerGraph Data Science Library](https://github.com/tigergraph/gsql-graph-algorithms) (which includes a variety of graph algorithms in addition to embeddings and can be installed with 1-click), then feel free to skip this step. Otherwise, we are going to install these two algorithms specifically for this demo.
1. Make sure you have ssh'ed into the TigerGraph server.
2. Go into the algorithms folder: `cd ~/embedding_demo/detect-cryptocurrency-fraud/algorithms`
3. Run the install script: `./install_algorithms.sh`
4. When the installation finishes, you can also see those queries in GraphStudio.

Now with both data loaded and algorithms installed, we can finally run the embedding algorithms now. On the same terminal inside the TigerGraph server,
1. Go to the repo directory: `cd ~/embedding_demo/detect-cryptocurrency-fraud`
2. Run the GSQL script to start both the fastRP and node2vec embedding jobs: `gsql run_embedding.gsql`. Note: the fastRP job takes a few minutes and the node2vec job can take hours. If you stopped it for any reason, you can open the script with any text editor, comment out the jobs that have already finished, and rerun the script. While it is running, you can monitor the resource usage at the [admin portal](http://localhost:14240/admin/#/dashboard) of GraphStudio.
3. Once the fastRP embedding finishes, you should see a file `fastRP_embedding.txt` under the output folder.
4. Once the node2vec embedding finishes, you should see a temporary file `paths.txt` under the tmp folder which records all the random walks generated, and the embedding file `node2vec_embedding.txt` under the output folder.
