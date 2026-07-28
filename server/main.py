from fastapi import FastAPI

app = FastAPI(
    title="Live Location Tracker API",
    version="1.0.0",
)


@app.get("/")
def root():
    return {
        "message": "Location Tracker backend is running",
        "status": "success",
    }


@app.get("/health")
def health_check():
    return {
        "status": "healthy",
    }