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
  int n;
  int size;
  vector<ll> sensors;
  vector<block_state> blocks;
  structB(int len, vector<ll> &initial){
    sensors=initial;
    n=len;
    size=ceil(sqrt(len));
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
        w.sum=(w.sum+sensors[i]%MOD+MOD)%MOD;
      }
    }
  }

  void push(int b){
    if(blocks[b].gain==1 && blocks[b].offset==0) return;
    int start=b*size;
    int end=min((b+1)*size,n);
    for(int i=start;i<end;i++){
      sensors[i]=blocks[b].gain*sensors[i]+blocks[b].offset;
    }
    blocks[b].gain=1;
    blocks[b].offset=0;
    return;
  }

  void rebuild(int b){
    int start=b*size;
    int end=min((b+1)*size,n);
    block_state& w=blocks[b];
    w.mn=sensors[start];
    w.mx=sensors[start];
    w.sum=sensors[start];
    for(int i=start+1;i<end;i++){
      w.mn=min(w.mn,sensors[i]);
      w.mx=max(w.mx,sensors[i]);
      w.sum=(w.sum+sensors[i]%MOD+MOD)%MOD;
    }
    return;
  }

  void comp_upd(int b, ll g, ll o){
    block_state& w=blocks[b];
    w.mn=w.mn*g+o;
    w.mx=w.mx*g+o;
    if(g<0) swap(w.mn,w.mx);
    w.sum=(w.sum*(g%MOD)+size*(o%MOD)+MOD)%MOD;
    w.gain=g*w.gain;
    w.offset=(g*w.offset)+o;
  }

  void update(int start, int end, ll g, ll o){
    int b1=start/size;
    int b2=end/size;
    for(int b=b1;b<=b2;b++){
      int l=b*size;
      int r=min((b+1)*size-1,n-1);
      if(l>=start && r<=end){
        comp_upd(b,g,o);
      } else {
        push(b);
        int s=max(start,l);
        int e=min(end,r);
        for(int i=s;i<=e;i++){
          sensors[i]=sensors[i]*g+o;
        }
        rebuild(b);
      }
    }
  }

  void display(){
    for(block_state q : blocks){
      cout<<q.mn<<" "<<q.mx<<" "<<q.sum<<"\n";
      cout<<"lazy : "<<q.gain<<" "<<q.offset<<"\n";
    }
    return;
  }
};

int main(){
  vector<ll> A(256,0);
  for(int i=0;i<256;i++) A[i]=i;
  A[0]=MOD;
  structB SQRTD=structB(256,A);
  SQRTD.display();
  cout<<"AFTER : \n";
  SQRTD.update(64,255,0,20);
  SQRTD.display();
  cout<<"FINAL : \n";
  SQRTD.update(65,255,1,100);
  SQRTD.display();
  return 0;
}
