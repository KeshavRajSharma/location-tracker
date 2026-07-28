from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from app.database import Base, engine
from app.routes.locations import router as locations_router

Base.metadata.create_all(bind=engine)

app = FastAPI(
    title="Live Location Tracker API",
    version="1.0.0",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=False,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(locations_router)


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