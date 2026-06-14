from collections import deque

class MaxFlowGraph:
    def __init__(self, node_num: int):
        self.node_num = node_num
        self.graph = [[] for _ in range(node_num)]

    def add_edge(self, from_node: int, to_node: int, cap: int = 1) -> None:
        self.graph[from_node].append({"to": to_node, "rev": len(self.graph[to_node]), "cap": cap})
        self.graph[to_node].append({"to": from_node, "rev": len(self.graph[from_node]) - 1, "cap": 0})

    def flow(self, source: int, target: int, flow_limit: int) -> bool:
        flow = 0
        while flow < flow_limit:
            level = self._bfs(source, target)
            if level[target] == -1:
                break
            next_edge = [0] * self.node_num
            while flow < flow_limit:
                new_flow = self._dfs(source, target, flow_limit - flow, level, next_edge)
                if not new_flow:
                    break
                flow += new_flow
        return flow >= flow_limit

    def _bfs(self, source: int, target: int) -> list[int]:
        level = [-1] * self.node_num
        level[source] = 0
        queue = deque([source])
        while queue:
            node = queue.popleft()
            for edge in self.graph[node]:
                next_node = edge["to"]
                if edge["cap"] <= 0 or level[next_node] >= 0:
                    continue
                level[next_node] = level[node] + 1
                if next_node == target:
                    return level
                queue.append(next_node)
        return level

    def _dfs(self, node: int, target: int, flow_limit: int, level: list[int], next_edge: list[int]) -> int:
        if node == target:
            return flow_limit
        while next_edge[node] < len(self.graph[node]):
            edge = self.graph[node][next_edge[node]]
            next_node = edge["to"]
            if edge["cap"] > 0 and level[next_node] == level[node] + 1:
                flow = self._dfs(next_node, target, min(flow_limit, edge["cap"]), level, next_edge)
                if flow:
                    edge["cap"] -= flow
                    self.graph[next_node][edge["rev"]]["cap"] += flow
                    return flow
            next_edge[node] += 1
        return 0


__all__ = ["MaxFlowGraph"]
