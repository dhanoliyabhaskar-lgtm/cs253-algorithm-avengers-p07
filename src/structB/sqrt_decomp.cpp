#include <iostream>
#include <vector>
#include <cmath>
using ll=long long;
int MOD=998244353;
using namespace std;

struct block_state{
  ll mn;
  ll mx;
  ll sum;
  ll gain=1;
  ll offset=0;
};

struct structB{
  vector<ll> sensors;
  vector<block_state> blocks;
  structB(int n, vector<ll> &initial){
    sensors=initial;
    int size=ceil(sqrt(n));
    blocks.resize(size);
    for(int i=0;i<n;i++){
      int b=i/size;
      block_state& w=blocks[b];
      if(i%size==0){
        w.mn=sensors[i];
        w.mx=sensors[i];
        w.sum=sensors[i]%MOD;
      } else{
        w.mn=min(w.mn,sensors[i]);
        w.mx=max(w.mx,sensors[i]);
        w.sum=(w.sum+sensors[i]%MOD)%MOD;
      }
    }
  }
  void display(){
    for(block_state q : blocks){
      cout<<q.mn<<" "<<q.mx<<" "<<q.sum<<"\n";
    }
    return;
  }
};

int main(){
  vector<ll> A(4,0);
  A[1]=1;
  A[2]=2;
  A[3]=3;
  structB SQRTD=structB(4,A);
  SQRTD.display();
  return 0;
}
