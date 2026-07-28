from fastapi import APIRouter, Depends, HTTPException, Response, status
from sqlalchemy.orm import Session

from app import models, schemas
from app.database import get_db

router = APIRouter(
    prefix="/locations",
    tags=["Locations"],
)


@router.post(
    "",
    response_model=schemas.LocationResponse,
    status_code=status.HTTP_201_CREATED,
)
def create_location(
    location: schemas.LocationCreate,
    db: Session = Depends(get_db),
):
    new_location = models.Location(
        device_id=location.device_id,
        latitude=location.latitude,
        longitude=location.longitude,
        speed=location.speed,
    )

    db.add(new_location)
    db.commit()
    db.refresh(new_location)

    return new_location


@router.get(
    "/latest",
    response_model=schemas.LocationResponse,
)
def get_latest_location(
    db: Session = Depends(get_db),
):
    location = (
        db.query(models.Location)
        .order_by(models.Location.recorded_at.desc())
        .first()
    )

    if location is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="No location data found",
        )

    return location


@router.get(
    "",
    response_model=list[schemas.LocationResponse],
)
def get_locations(
    db: Session = Depends(get_db),
):
    return (
        db.query(models.Location)
        .order_by(models.Location.recorded_at.asc())
        .all()
    )


@router.delete(
    "",
    status_code=status.HTTP_204_NO_CONTENT,
)
def delete_locations(
    db: Session = Depends(get_db),
):
    db.query(models.Location).delete()
    db.commit()

    return Response(status_code=status.HTTP_204_NO_CONTENT)