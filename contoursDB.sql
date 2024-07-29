-- Database: contours

-- DROP DATABASE IF EXISTS contours;

CREATE DATABASE contours
    WITH
    OWNER = postgres
    ENCODING = 'UTF8'
    LC_COLLATE = 'Russian_Russia.utf8'
    LC_CTYPE = 'Russian_Russia.utf8'
    LOCALE_PROVIDER = 'libc'
    TABLESPACE = pg_default
    CONNECTION LIMIT = -1
    IS_TEMPLATE = False;

-- Table: public.contours

-- DROP TABLE IF EXISTS public.contours;

CREATE TABLE IF NOT EXISTS public.contours
(
    id_pk integer NOT NULL DEFAULT nextval('contours_id_pk_seq1'::regclass),
    image_name character varying COLLATE pg_catalog."default",
    contour_numbers integer[],
    contour_names character varying[] COLLATE pg_catalog."default",
    CONSTRAINT contours_pkey1 PRIMARY KEY (id_pk)
)

TABLESPACE pg_default;

ALTER TABLE IF EXISTS public.contours
    OWNER to postgres;

-- SEQUENCE: public.contours_id_pk_seq1

-- DROP SEQUENCE IF EXISTS public.contours_id_pk_seq1;

CREATE SEQUENCE IF NOT EXISTS public.contours_id_pk_seq1
    INCREMENT 1
    START 1
    MINVALUE 1
    MAXVALUE 2147483647
    CACHE 1;

ALTER SEQUENCE public.contours_id_pk_seq1
    OWNED BY public.contours.id_pk;

ALTER SEQUENCE public.contours_id_pk_seq1
    OWNER TO postgres;