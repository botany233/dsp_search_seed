// This file is derived from https://github.com/atcoder/ac-library, originally released under CC0-1.0.
#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <queue>
#include <vector>

template <class T> struct simple_queue {
	std::vector<T> payload;
	int pos = 0;
	void reserve(int n) {
		payload.reserve(n);
	}
	int size() const {
		return int(payload.size()) - pos;
	}
	bool empty() const {
		return pos == int(payload.size());
	}
	void push(const T& t) {
		payload.push_back(t);
	}
	T& front() {
		return payload[pos];
	}
	void clear() {
		payload.clear();
		pos = 0;
	}
	void pop() {
		pos++;
	}
};

class MaxFlowGraph {
public:
	explicit MaxFlowGraph(int n): _n(n),g(n) {}

	int add_edge(int from,int to,int cap = 1) {
		int m = int(pos.size());
		pos.push_back({from,int(g[from].size())});
		int from_id = int(g[from].size());
		int to_id = int(g[to].size());
		if(from == to) to_id++;
		g[from].push_back(_edge{to,to_id,cap});
		g[to].push_back(_edge{from,from_id,0});
		return m;
	}

	struct edge {
		int from,to;
		int cap,flow;
	};

	bool flow(int s,int t,int flow_limit) {
		std::vector<int> level(_n),iter(_n);
		simple_queue<int> que;

		auto bfs = [&]() {
			std::fill(level.begin(),level.end(),-1);
			level[s] = 0;
			que.clear();
			que.push(s);
			while(!que.empty()) {
				int v = que.front();
				que.pop();
				for(auto e : g[v]) {
					if(e.cap == 0 || level[e.to] >= 0) continue;
					level[e.to] = level[v] + 1;
					if(e.to == t) return;
					que.push(e.to);
				}
			}
		};

		auto dfs = [&](auto self,int v,int up) {
			if(v == s) return up;
			int res = 0;
			int level_v = level[v];
			for(int& i = iter[v]; i < int(g[v].size()); i++) {
				_edge& e = g[v][i];
				if(level_v <= level[e.to] || g[e.to][e.rev].cap == 0) continue;
				int d = self(self,e.to,std::min(up - res,g[e.to][e.rev].cap));
				if(d <= 0) continue;
				g[v][i].cap += d;
				g[e.to][e.rev].cap -= d;
				res += d;
				if(res == up) return res;
			}
			level[v] = _n;
			return res;
		};

		int flow = 0;
		while(flow < flow_limit) {
			bfs();
			if(level[t] == -1) break;
			std::fill(iter.begin(),iter.end(),0);
			int f = dfs(dfs,t,flow_limit - flow);
			if(!f) break;
			flow += f;
		}
		return flow >= flow_limit;
	}

private:
	int _n;
	struct _edge {
		int to,rev;
		int cap;
	};
	std::vector<std::pair<int,int>> pos;
	std::vector<std::vector<_edge>> g;
};
