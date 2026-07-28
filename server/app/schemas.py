from datetime import datetime

from pydantic import BaseModel, ConfigDict, Field


class LocationCreate(BaseModel):
    device_id: str = Field(
        min_length=1,
        max_length=50,
    )

    latitude: float = Field(
        ge=-90,
        le=90,
    )

    longitude: float = Field(
        ge=-180,
        le=180,
    )

    speed: float = Field(
        default=0.0,
        ge=0,
    )


class LocationResponse(LocationCreate):
    id: int
    recorded_at: datetime

    model_config = ConfigDict(from_attributes=True)