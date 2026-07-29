from fastapi import WebSocket


class ConnectionManager:
    def __init__(self) -> None:
        self.active_connections: list[WebSocket] = []

    async def connect(self, websocket: WebSocket) -> None:
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket) -> None:
        if websocket in self.active_connections:
            self.active_connections.remove(websocket)

    async def broadcast_json(self, data: dict) -> None:
        disconnected_connections: list[WebSocket] = []

        for connection in self.active_connections:
            try:
                await connection.send_json(data)
            except Exception:
                disconnected_connections.append(connection)

        for connection in disconnected_connections:
            self.disconnect(connection)


manager = ConnectionManager()