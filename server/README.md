# Location Tracker — Server

FastAPI backend for the live GPS location tracker project.

## Stack

- **FastAPI** 0.140
- **SQLAlchemy** 2.0 (ORM)
- **SQLite** (local database — created automatically at runtime)
- **Pydantic** 2 (request/response validation)
- **Uvicorn** (ASGI server)

## Project structure

```
server/
├── main.py          # FastAPI app, startup, route registration
├── database.py      # SQLAlchemy engine and session setup (stub)
├── models.py        # SQLAlchemy ORM models (stub)
├── schemas.py       # Pydantic request/response schemas (stub)
├── requirements.txt # Pinned Python dependencies
└── venv/            # Local virtual environment (git-ignored)
```

## Running locally

```bash
# Create and activate virtual environment (first time only)
python -m venv venv
source venv/bin/activate          # Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Start the development server
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

The API will be available at:
- `http://localhost:8000/`         → health check root
- `http://localhost:8000/health`   → health check endpoint
- `http://localhost:8000/docs`     → Swagger UI (auto-generated)
- `http://localhost:8000/redoc`    → ReDoc (auto-generated)
